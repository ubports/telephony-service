/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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

#include <TelepathyQt/PendingOperation>
#include <QTimer>
#include "accountentry.h"
#include "telepathyhelper.h"

AccountEntry::AccountEntry(const Tp::AccountPtr &account, QObject *parent) :
    QObject(parent), mAccount(account)
{
    initialize();
}

QString AccountEntry::accountId() const
{
    if (mAccount.isNull()) {
        return QString::null;
    }

    return mAccount->uniqueIdentifier();
}

bool AccountEntry::active() const
{
    return (!mAccount.isNull() &&
            !mAccount->connection().isNull() &&
            !mAccount->connection()->selfContact().isNull() &&
             mAccount->connection()->selfContact()->presence().type() != Tp::ConnectionPresenceTypeOffline);
}

QString AccountEntry::displayName() const
{
    if (mAccount.isNull()) {
        return QString::null;
    }

    return mAccount->displayName();
}

QString AccountEntry::status() const
{
    if (mAccount.isNull() || mAccount->connection().isNull() || mAccount->connection()->selfContact().isNull()) {
        return QString::null;
    }
    Tp::Presence presence = mAccount->connection()->selfContact()->presence();
    return presence.status();
}

QString AccountEntry::statusMessage() const
{
    if (mAccount.isNull() || mAccount->connection().isNull() || mAccount->connection()->selfContact().isNull()) {
        return QString::null;
    }
    Tp::Presence presence = mAccount->connection()->selfContact()->presence();
    return presence.statusMessage();
}

QString AccountEntry::selfContactId() const
{
    if (!mAccount.isNull() && !mAccount->connection().isNull() &&
           !mAccount->connection()->selfContact().isNull()) {
        return mAccount->connection()->selfContact()->id();
    }
    return QString();
}

void AccountEntry::setDisplayName(const QString &name)
{
    if (mAccount.isNull()) {
        return;
    }
    mAccount->setDisplayName(name);
}

bool AccountEntry::connected() const
{
    return !mAccount.isNull() && !mAccount->connection().isNull() &&
           !mAccount->connection()->selfContact().isNull() &&
            mAccount->connection()->selfContact()->presence().type() == Tp::ConnectionPresenceTypeAvailable;
}

Tp::AccountPtr AccountEntry::account() const
{
    return mAccount;
}

AccountEntry::AccountType AccountEntry::type() const
{
    return GenericAccount;
}

QStringList AccountEntry::addressableVCardFields() const
{
    return mAccount->protocolInfo().addressableVCardFields();
}

bool AccountEntry::compareIds(const QString &first, const QString &second) const
{
    return first == second;
}

void AccountEntry::initialize()
{
    if (mAccount.isNull()) {
        return;
    }

    // propagate the display name changes
    connect(mAccount.data(),
            SIGNAL(removed()),
            SIGNAL(removed()));
 
    // propagate the display name changes
    connect(mAccount.data(),
            SIGNAL(displayNameChanged(QString)),
            SIGNAL(displayNameChanged()));
    \
    // watch for account state and connection changes
    connect(mAccount.data(), &Tp::Account::stateChanged, [this](bool enabled) {
        if (!enabled) {
            ensureEnabled();
        }
    });

    connect(mAccount.data(),
            SIGNAL(connectionChanged(Tp::ConnectionPtr)),
            SLOT(onConnectionChanged()));

    connect(this,
            SIGNAL(connectedChanged()),
            SIGNAL(activeChanged()));

    // and make sure it is enabled and connected
    if (!mAccount->isEnabled()) {
        QTimer::singleShot(0, this, SLOT(ensureEnabled()));
    } else {
        QTimer::singleShot(0, this, SLOT(ensureConnected()));
    }
    Q_EMIT accountIdChanged();
}

void AccountEntry::ensureEnabled()
{
    mAccount->setConnectsAutomatically(true);
    connect(mAccount->setEnabled(true),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(ensureConnected()));
}

void AccountEntry::ensureConnected()
{
    // if the account is not connected, request it to connect
    if (!mAccount->connection() || mAccount->connectionStatus() != Tp::ConnectionStatusConnected) {
        Tp::Presence presence(Tp::ConnectionPresenceTypeAvailable, "available", "online");
        mAccount->setRequestedPresence(presence);
    } else {
        onConnectionChanged();
    }

    Q_EMIT accountReady();
}

void AccountEntry::watchSelfContactPresence()
{
    if (mAccount.isNull() || mAccount->connection().isNull()) {
        return;
    }

    connect(mAccount->connection()->selfContact().data(),
            SIGNAL(presenceChanged(Tp::Presence)),
            SIGNAL(connectedChanged()));
    connect(mAccount->connection()->selfContact().data(),
            SIGNAL(presenceChanged(Tp::Presence)),
            SIGNAL(statusMessageChanged()));
    connect(mAccount->connection()->selfContact().data(),
            SIGNAL(presenceChanged(Tp::Presence)),
            SIGNAL(statusChanged()));
}

void AccountEntry::onSelfHandleChanged(uint handle)
{
    Q_UNUSED(handle)
    watchSelfContactPresence();

    Q_EMIT statusChanged();
    Q_EMIT statusMessageChanged();
    Q_EMIT connectedChanged();
    Q_EMIT selfContactIdChanged();
}

void AccountEntry::onConnectionChanged()
{
    if (!mAccount->connection()) {
        // ensure the account gets connected
        ensureConnected();
    } else {
        mConnectionInfo.busName = mAccount->connection()->busName();
        mConnectionInfo.objectPath = mAccount->connection()->objectPath();

        connect(mAccount->connection().data(),
                SIGNAL(selfHandleChanged(uint)),
                SLOT(onSelfHandleChanged(uint)));

        watchSelfContactPresence();
    }

    Q_EMIT connectedChanged();
    Q_EMIT selfContactIdChanged();
}
