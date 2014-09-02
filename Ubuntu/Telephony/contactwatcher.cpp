/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "contactwatcher.h"
#include "contactutils.h"
#include "phoneutils.h"
#include <QContactManager>
#include <QContactFetchByIdRequest>
#include <QContactFetchRequest>
#include <QContactAvatar>
#include <QContactDetailFilter>
#include <QContactPhoneNumber>

namespace C {
#include <libintl.h>
}

ContactWatcher::ContactWatcher(QObject *parent) :
    QObject(parent), mRequest(0), mInteractive(false), mCompleted(false)
{
    connect(ContactUtils::sharedManager(),
            SIGNAL(contactsAdded(QList<QContactId>)),
            SLOT(onContactsAdded(QList<QContactId>)));
    connect(ContactUtils::sharedManager(),
            SIGNAL(contactsChanged(QList<QContactId>)),
            SLOT(onContactsChanged(QList<QContactId>)));
    connect(ContactUtils::sharedManager(),
            SIGNAL(contactsRemoved(QList<QContactId>)),
            SLOT(onContactsRemoved(QList<QContactId>)));
}

ContactWatcher::~ContactWatcher()
{
    if (mRequest) {
        mRequest->cancel();
        delete mRequest;
    }
}

void ContactWatcher::searchByPhoneNumber(const QString &phoneNumber)
{
    if (!mCompleted) {
        // componenty is not ready yet
        return;
    }

    // cancel current request if necessary
    if (mRequest) {
        mRequest->cancel();
        mRequest->deleteLater();
    }

    mRequest = new QContactFetchRequest(this);
    mRequest->setFilter(QContactPhoneNumber::match(phoneNumber));
    connect(mRequest, SIGNAL(stateChanged(QContactAbstractRequest::State)),
                      SLOT(onRequestStateChanged(QContactAbstractRequest::State)));
    connect(mRequest, SIGNAL(resultsAvailable()), SLOT(resultsAvailable()));
    mRequest->setManager(ContactUtils::sharedManager());
    mRequest->start();
}

QString ContactWatcher::contactId() const
{
    QString id = mContactId.toString();
    if (id == QStringLiteral("qtcontacts:::")) {
        return QString();
    } else {
        return id;
    }
}

QString ContactWatcher::avatar() const
{
    return mAvatar;
}

QString ContactWatcher::alias() const
{
    return mAlias;
}

QString ContactWatcher::phoneNumber() const
{
    return mPhoneNumber;
}

QList<int> ContactWatcher::phoneNumberSubTypes() const
{
    return mPhoneNumberSubTypes;
}

QList<int> ContactWatcher::phoneNumberContexts() const
{
    return mPhoneNumberContexts;
}

void ContactWatcher::setPhoneNumber(const QString &phoneNumber)
{
    if (mPhoneNumber == phoneNumber) {
        return;
    }

    const bool isPrivate = phoneNumber.startsWith("x-ofono-private");
    const bool isUnknown = phoneNumber.startsWith("x-ofono-unknown");
    const bool isInteractive = !phoneNumber.isEmpty() && !isPrivate && !isUnknown;

    mPhoneNumber = phoneNumber;
    Q_EMIT phoneNumberChanged();

    if (mPhoneNumber.isEmpty() || isPrivate || isUnknown) {
        mAlias.clear();
        mContactId = QContactId();
        mAvatar.clear();
        mPhoneNumberSubTypes.clear();
        mPhoneNumberContexts.clear();

        if (isPrivate) {
            mAlias = C::gettext("Private Number");
        } else if (isUnknown) {
            mAlias = C::gettext("Unknown Number");
        }

        Q_EMIT contactIdChanged();
        Q_EMIT avatarChanged();
        Q_EMIT aliasChanged();
        Q_EMIT phoneNumberSubTypesChanged();
        Q_EMIT phoneNumberContextsChanged();
        Q_EMIT isUnknownChanged();
    } else {
        searchByPhoneNumber(mPhoneNumber);
    }

    if (isInteractive != mInteractive) {
        mInteractive = isInteractive;
        Q_EMIT interactiveChanged();
    }
}

bool ContactWatcher::isUnknown() const
{
    return mContactId.isNull();
}

bool ContactWatcher::interactive() const
{
    return mInteractive;
}

void ContactWatcher::classBegin()
{
}

void ContactWatcher::componentComplete()
{
    mCompleted = true;
    // query for phone if the phone number was initialized
    if (!mPhoneNumber.isEmpty()) {
        searchByPhoneNumber(mPhoneNumber);
    }
}

void ContactWatcher::onContactsAdded(QList<QContactId> ids)
{
    // ignore this signal if we have a contact already
    // or if we have no phone number set
    if (!mContactId.isNull() || mPhoneNumber.isEmpty()) {
        return;
    }

    searchByPhoneNumber(mPhoneNumber);
}

void ContactWatcher::onContactsChanged(QList<QContactId> ids)
{
    // check for changes even if we have this contact already,
    // as the number might have changed, thus invalidating the current contact
    if (!mPhoneNumber.isEmpty() || ids.contains(mContactId)) {
        searchByPhoneNumber(mPhoneNumber);
    }
}

void ContactWatcher::onContactsRemoved(QList<QContactId> ids)
{
    // if the current contact got removed, clear it before trying to search for a new one
    if (ids.contains(mContactId)) {
        mAlias.clear();
        mContactId = QContactId();
        mAvatar.clear();
        mPhoneNumberSubTypes.clear();
        mPhoneNumberContexts.clear();
        Q_EMIT contactIdChanged();
        Q_EMIT avatarChanged();
        Q_EMIT aliasChanged();
        Q_EMIT phoneNumberSubTypesChanged();
        Q_EMIT phoneNumberContextsChanged();
        Q_EMIT isUnknownChanged();

        if (!mPhoneNumber.isEmpty()) {
            searchByPhoneNumber(mPhoneNumber);
        }
    }
}

void ContactWatcher::resultsAvailable()
{
    QContactFetchRequest *request = qobject_cast<QContactFetchRequest*>(sender());
    if (request && request->contacts().size() > 0) {
        // use the first match
        QContact contact = request->contacts().at(0);
        if (contact.id() != mContactId) {
            mContactId = contact.id();
            Q_EMIT contactIdChanged();
        }

        QString newAvatar = contact.detail<QContactAvatar>().imageUrl().toString();
        if (newAvatar != mAvatar) {
            mAvatar = newAvatar;
            Q_EMIT avatarChanged();
        }

        QString newAlias = ContactUtils::formatContactName(contact);
        if (newAlias != mAlias) {
            mAlias = newAlias;
            Q_EMIT aliasChanged();
            Q_EMIT isUnknownChanged();
        }

        Q_FOREACH(const QContactPhoneNumber phoneNumber, contact.details(QContactDetail::TypePhoneNumber)) {
            if (PhoneUtils::comparePhoneNumbers(phoneNumber.number(), mPhoneNumber)) {
                QList<int> newSubTypes = phoneNumber.subTypes();
                if (newSubTypes != mPhoneNumberSubTypes) {
                    mPhoneNumberSubTypes = phoneNumber.subTypes();
                    Q_EMIT phoneNumberSubTypesChanged();
                }
                QList<int> newContexts = phoneNumber.contexts();
                if (newContexts != mPhoneNumberContexts) {
                    mPhoneNumberContexts =  newContexts;
                    Q_EMIT phoneNumberContextsChanged();
                }
            }
        }
    }
}

void ContactWatcher::onRequestStateChanged(QContactAbstractRequest::State state)
{
    QContactFetchRequest *request = mRequest;
    if (request && state == QContactAbstractRequest::FinishedState) {
        mRequest = 0;
        request->deleteLater();

        // if we got no results and we had a contact previously, we need to clear the data
        if (request->contacts().isEmpty() && !mContactId.isNull()) {
            mAlias.clear();
            mContactId = QContactId();
            mAvatar.clear();
            mPhoneNumberSubTypes.clear();
            mPhoneNumberContexts.clear();

            Q_EMIT contactIdChanged();
            Q_EMIT avatarChanged();
            Q_EMIT aliasChanged();
            Q_EMIT phoneNumberSubTypesChanged();
            Q_EMIT phoneNumberContextsChanged();
            Q_EMIT isUnknownChanged();
        }
    }
}
