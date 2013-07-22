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
#include <QContactManager>
#include <QContactFetchByIdRequest>
#include <QContactFetchRequest>
#include <QContactAvatar>
#include <QContactDisplayLabel>
#include <QContactDetailFilter>
#include <QContactPhoneNumber>

QContactManager *ContactWatcher::engineInstance()
{
    static QContactManager* manager = new QContactManager("galera");
    return manager;
}

ContactWatcher::ContactWatcher(QObject *parent) :
    QObject(parent)
{
    mContactManager = engineInstance();
    connect(mContactManager,
            SIGNAL(contactsAdded(QList<QContactId>)),
            SLOT(onContactsAdded(QList<QContactId>)));
    connect(mContactManager,
            SIGNAL(contactsChanged(QList<QContactId>)),
            SLOT(onContactsChanged(QList<QContactId>)));
    connect(mContactManager,
            SIGNAL(contactsRemoved(QList<QContactId>)),
            SLOT(onContactsRemoved(QList<QContactId>)));
}

void ContactWatcher::searchByPhoneNumber(const QString &phoneNumber)
{
    QContactFetchRequest *request = new QContactFetchRequest(this);
    request->setFilter(QContactPhoneNumber::match(phoneNumber));
    connect(request, SIGNAL(stateChanged(QContactAbstractRequest::State)), SLOT(onRequestStateChanged(QContactAbstractRequest::State)));
    connect(request, SIGNAL(resultsAvailable()), SLOT(resultsAvailable()));
    request->setManager(mContactManager);
    request->start();
}

QString ContactWatcher::contactId() const
{
    return mContactId;
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

void ContactWatcher::setPhoneNumber(const QString &phoneNumber)
{
    qDebug() << "setPhoneNumber" << phoneNumber;
    mPhoneNumber = phoneNumber;
    if (phoneNumber.isEmpty()) {
        mAlias.clear();
        mContactId.clear();
        mAvatar.clear();
        Q_EMIT contactIdChanged();
        Q_EMIT avatarChanged();
        Q_EMIT aliasChanged();
        Q_EMIT isUnknownChanged();
        return;
    }

    searchByPhoneNumber(mPhoneNumber);
}

bool ContactWatcher::isUnknown() const
{
    return mContactId.isEmpty();
}


void ContactWatcher::onContactsAdded(QList<QContactId> ids)
{
    // ignore this signal if we have a contact already
    // or if we have no phone number set
    if (!mContactId.isEmpty() || mPhoneNumber.isEmpty())
        return;

    searchByPhoneNumber(mPhoneNumber);
}

void ContactWatcher::onContactsChanged(QList<QContactId> ids)
{
    if (!mContactId.isEmpty() && ids.contains(QContactId::fromString(mContactId)) && !mPhoneNumber.isEmpty()) {
        searchByPhoneNumber(mPhoneNumber);
    }
}

void ContactWatcher::onContactsRemoved(QList<QContactId> ids)
{
    if (!mContactId.isEmpty() && ids.contains(QContactId::fromString(mContactId)) && !mPhoneNumber.isEmpty()) {
        // this contact got removed, so check if we have another one that matches this phoneNumber
        searchByPhoneNumber(mPhoneNumber);
    } else {
        mAlias.clear();
        mContactId.clear();
        mAvatar.clear();
        Q_EMIT contactIdChanged();
        Q_EMIT avatarChanged();
        Q_EMIT aliasChanged();
        Q_EMIT isUnknownChanged();
    }
}

void ContactWatcher::resultsAvailable()
{
    qDebug() << "resultsAvailable";
    QContactFetchRequest *request = qobject_cast<QContactFetchRequest*>(sender());
    if (request && request->contacts().size() > 0) {
        QContact contact = request->contacts().at(0);
        mContactId = contact.id().toString();
        mAvatar = QContactAvatar(contact.detail(QContactDetail::TypeAvatar)).imageUrl().toString();
        mAlias = QContactDisplayLabel(contact.detail(QContactDetail::TypeDisplayLabel)).label();
        qDebug() << mContactId << mAvatar << mAlias;
    } else {
        qDebug() << "no contacts found for number" << mPhoneNumber;
        mAlias.clear();
        mContactId.clear();
        mAvatar.clear();
    }

    Q_EMIT contactIdChanged();
    Q_EMIT avatarChanged();
    Q_EMIT aliasChanged();
    Q_EMIT isUnknownChanged();
}

void ContactWatcher::onRequestStateChanged(QContactAbstractRequest::State state)
{
    qDebug() << "requestChanged";
    QContactFetchRequest *request = qobject_cast<QContactFetchRequest*>(sender());
    if (request && state == QContactAbstractRequest::FinishedState) {
        request->deleteLater();
    }
}
