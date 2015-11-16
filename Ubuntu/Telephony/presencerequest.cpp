/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include "presencerequest.h"
#include "telepathyhelper.h"

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>

PresenceRequest::PresenceRequest(QObject *parent) :
    QObject(parent), mCompleted(false)
{
    connect(TelepathyHelper::instance(), SIGNAL(accountAdded(AccountEntry*)), SLOT(onAccountAdded(AccountEntry*)));
}

PresenceRequest::~PresenceRequest()
{
}

void PresenceRequest::onAccountAdded(AccountEntry *account)
{
    if (account->accountId() == mAccountId) {
        startPresenceRequest();
    }
}

void PresenceRequest::startPresenceRequest()
{
    if (!mCompleted || mIdentifier.isEmpty() || mAccountId.isEmpty()) {
        // component is not ready yet
        return;
    }

    AccountEntry *account = TelepathyHelper::instance()->accountForId(mAccountId);
    if (!account || account->account()->connection().isNull()) {
        mContact.reset();
        onPresenceChanged();
        return;
    }
 
    Tp::ContactManagerPtr contactManager = account->account()->connection()->contactManager();
    Tp::PendingContacts *pendingContact = contactManager->contactsForIdentifiers(QStringList() << mIdentifier);
    connect(pendingContact,
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onContactReceived(Tp::PendingOperation*)));
}

void PresenceRequest::onContactReceived(Tp::PendingOperation *op)
{
    Tp::PendingContacts *contacts = qobject_cast<Tp::PendingContacts *>(op);
    if (!contacts || !contacts->isValid() || contacts->contacts().length() != 1) {
        return;
    }

    if (mContact) {
        disconnect(mContact.data(), 0,0,0);
    }

    mContact = contacts->contacts()[0];
    connect(mContact.data(), SIGNAL(presenceChanged(const Tp::Presence &)), this, SLOT(onPresenceChanged()));
    onPresenceChanged();
}

void PresenceRequest::onPresenceChanged()
{
    Q_EMIT statusChanged();
    Q_EMIT statusMessageChanged();
    Q_EMIT typeChanged();
}

uint PresenceRequest::type() const
{
    if (mContact) {
        return mContact->presence().type();
    }

    return PresenceTypeUnset;
}

QString PresenceRequest::status() const
{
    if (mContact) {
        return mContact->presence().status();
    }
    return QString();
}

QString PresenceRequest::statusMessage() const
{
    if (mContact) {
        return mContact->presence().statusMessage();
    }
    return QString();
}

QString PresenceRequest::accountId() const
{
    return mAccountId;
}

QString PresenceRequest::identifier() const
{
    return mIdentifier;
}

void PresenceRequest::setIdentifier(const QString &identifier)
{
    if (mIdentifier == identifier) {
        return;
    }

    mIdentifier = identifier;

    startPresenceRequest();
}

void PresenceRequest::setAccountId(const QString &accountId)
{
    if (mAccountId == accountId) {
        return;
    }

    mAccountId = accountId;

    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
    if (!account) {
        return;
    }

    connect(account, SIGNAL(connectedChanged()), this, SLOT(startPresenceRequest()));

    startPresenceRequest();
}

void PresenceRequest::classBegin()
{
}

void PresenceRequest::componentComplete()
{
    mCompleted = true;
    startPresenceRequest();
}

