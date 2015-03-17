/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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
#include "accountentry.h"
#include "telepathyhelper.h"
#include <QContactManager>
#include <QContactFetchByIdRequest>
#include <QContactFetchRequest>
#include <QContactAvatar>
#include <QContactExtendedDetail>
#include <QContactPhoneNumber>
#include <QContactDetailFilter>
#include <QContactIntersectionFilter>
#include <QContactUnionFilter>

namespace C {
#include <libintl.h>
}

ContactWatcher::ContactWatcher(QObject *parent) :
    QObject(parent), mRequest(0), mInteractive(false), mCompleted(false)
{
    // addressable VCard fields defaults to "tel" only
    mAddressableFields << "tel";
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

void ContactWatcher::startSearching()
{
    if (!mCompleted || mIdentifier.isEmpty()) {
        // componenty is not ready yet or no identifier given
        return;
    }

    // cancel current request if necessary
    if (mRequest) {
        mRequest->cancel();
        mRequest->deleteLater();
    }

    // FIXME: search for all the fields
    mRequest = new QContactFetchRequest(this);

    QContactUnionFilter topLevelFilter;
    Q_FOREACH(const QString &field, mAddressableFields) {
        if (field == "tel") {
            topLevelFilter.append(QContactPhoneNumber::match(mIdentifier));
        } else {
            // FIXME: handle more fields
            // rely on a generic field filter
            QContactDetailFilter nameFilter = QContactDetailFilter();
            nameFilter.setDetailType(QContactExtendedDetail::Type, QContactExtendedDetail::FieldName);
            nameFilter.setMatchFlags(QContactFilter::MatchExactly);
            nameFilter.setValue(field);

            QContactDetailFilter valueFilter = QContactDetailFilter();
            valueFilter.setDetailType(QContactExtendedDetail::Type, QContactExtendedDetail::FieldData);
            valueFilter.setMatchFlags(QContactFilter::MatchExactly);
            valueFilter.setValue(mIdentifier);

            QContactIntersectionFilter intersectionFilter;
            intersectionFilter.append(nameFilter);
            intersectionFilter.append(valueFilter);

            topLevelFilter.append(intersectionFilter);
        }
    }

    mRequest->setFilter(topLevelFilter);
    connect(mRequest, SIGNAL(stateChanged(QContactAbstractRequest::State)),
                      SLOT(onRequestStateChanged(QContactAbstractRequest::State)));
    connect(mRequest, SIGNAL(resultsAvailable()), SLOT(onResultsAvailable()));
    mRequest->setManager(ContactUtils::sharedManager());
    mRequest->start();
}

QVariantList ContactWatcher::wrapIntList(const QList<int> &list)
{
    QVariantList resultList;
    Q_FOREACH(int value, list) {
        resultList << value;
    }
    return resultList;
}

QList<int> ContactWatcher::unwrapIntList(const QVariantList &list)
{
    QList<int> resultList;
    Q_FOREACH(const QVariant &value, list) {
        resultList << value.toInt();
    }
    return resultList;
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

QString ContactWatcher::identifier() const
{
    return mIdentifier;
}

void ContactWatcher::setIdentifier(const QString &identifier)
{
    if (mIdentifier == identifier) {
        return;
    }

    // FIXME: ofono stuff, maybe move somewhere else?
    const bool isPrivate = identifier.startsWith("x-ofono-private");
    const bool isUnknown = identifier.startsWith("x-ofono-unknown");
    const bool isInteractive = !identifier.isEmpty() && !isPrivate && !isUnknown;

    mIdentifier = identifier;
    Q_EMIT identifierChanged();

    if (mIdentifier.isEmpty() || isPrivate || isUnknown) {
        mAlias.clear();
        mContactId = QContactId();
        mAvatar.clear();
        mDetailProperties.clear();

        if (isPrivate) {
            mAlias = C::gettext("Private Number");
        } else if (isUnknown) {
            mAlias = C::gettext("Unknown Number");
        }

        Q_EMIT contactIdChanged();
        Q_EMIT avatarChanged();
        Q_EMIT aliasChanged();
        Q_EMIT detailPropertiesChanged();
        Q_EMIT isUnknownChanged();
    } else {
        startSearching();
    }

    if (isInteractive != mInteractive) {
        mInteractive = isInteractive;
        Q_EMIT interactiveChanged();
    }
}

QVariantMap ContactWatcher::detailProperties() const
{
    return mDetailProperties;
}

bool ContactWatcher::isUnknown() const
{
    return mContactId.isNull();
}

bool ContactWatcher::interactive() const
{
    return mInteractive;
}

QStringList ContactWatcher::addressableFields() const
{
    return mAddressableFields;
}

void ContactWatcher::setAddressableFields(const QStringList &fields)
{
    mAddressableFields = fields;
    Q_EMIT addressableFieldsChanged();

    startSearching();
}

void ContactWatcher::classBegin()
{
}

void ContactWatcher::componentComplete()
{
    mCompleted = true;
    startSearching();
}

void ContactWatcher::onContactsAdded(QList<QContactId> ids)
{
    // ignore this signal if we have a contact already
    // or if we have no phone number set
    if (!mContactId.isNull() || mIdentifier.isEmpty()) {
        return;
    }

    startSearching();
}

void ContactWatcher::onContactsChanged(QList<QContactId> ids)
{
    // check for changes even if we have this contact already,
    // as the number might have changed, thus invalidating the current contact
    startSearching();
}

void ContactWatcher::onContactsRemoved(QList<QContactId> ids)
{
    // if the current contact got removed, clear it before trying to search for a new one
    if (ids.contains(mContactId)) {
        mAlias.clear();
        mContactId = QContactId();
        mAvatar.clear();
        mDetailProperties.clear();
        Q_EMIT contactIdChanged();
        Q_EMIT avatarChanged();
        Q_EMIT aliasChanged();
        Q_EMIT detailPropertiesChanged();
        Q_EMIT isUnknownChanged();

        startSearching();
    }
}

void ContactWatcher::onResultsAvailable()
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

        Q_FOREACH(const QString &field, mAddressableFields) {
            if (field == "tel") {
                Q_FOREACH(const QContactPhoneNumber phoneNumber, contact.details(QContactDetail::TypePhoneNumber)) {
                    if (PhoneUtils::comparePhoneNumbers(phoneNumber.number(), mIdentifier)) {
                        mDetailProperties["type"] = (int)QContactDetail::TypePhoneNumber;
                        mDetailProperties["phoneNumberSubTypes"] = wrapIntList(phoneNumber.subTypes());
                        mDetailProperties["phoneNumberContexts"] = wrapIntList(phoneNumber.contexts());
                        Q_EMIT detailPropertiesChanged();
                        break;
                    }
                }
            } else {
                // FIXME: add proper support for more fields
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
            mDetailProperties.clear();

            Q_EMIT contactIdChanged();
            Q_EMIT avatarChanged();
            Q_EMIT aliasChanged();
            Q_EMIT detailPropertiesChanged();
            Q_EMIT isUnknownChanged();
        }
    }
}
