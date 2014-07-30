/*
 * Copyright (C) 2013 Canonical, Ltd.
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

QString AccountEntry::displayName() const
{
    if (mAccount.isNull()) {
        return QString::null;
    }

    return mAccount->displayName();
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
            mAccount->connection()->selfContact()->presence().type() == Tp::ConnectionPresenceTypeAvailable;
}

QStringList AccountEntry::emergencyNumbers() const
{
    return mEmergencyNumbers;
}

QString AccountEntry::voicemailNumber() const
{
    return mVoicemailNumber;
}

Tp::AccountPtr AccountEntry::account() const
{
    return mAccount;
}

void AccountEntry::initialize()
{
    if (mAccount.isNull()) {
        return;
    }

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
}

void AccountEntry::onConnectionChanged()
{
    QDBusConnection dbusConnection = QDBusConnection::sessionBus();
    if (!mAccount->connection()) {
        // disconnect any previous dbus connections
        if (!mConnectionInfo.objectPath.isEmpty()) {
            dbusConnection.disconnect(mConnectionInfo.busName, mConnectionInfo.objectPath,
                                      CANONICAL_TELEPHONY_EMERGENCYMODE_IFACE, "EmergencyNumbersChanged",
                                      this, SLOT(onEmergencyNumbersChanged(QStringList)));
        }

        // and ensure the account gets connected
        ensureConnected();
    } else {
        mConnectionInfo.busName = mAccount->connection()->busName();
        mConnectionInfo.objectPath = mAccount->connection()->objectPath();

        // connect the emergency numbers changed signal
        dbusConnection.connect(mConnectionInfo.busName, mConnectionInfo.objectPath,
                               CANONICAL_TELEPHONY_EMERGENCYMODE_IFACE, "EmergencyNumbersChanged",
                               this, SLOT(onEmergencyNumbersChanged(QStringList)));

        // and get the current value of the emergency numbers
        QDBusInterface connIface(mConnectionInfo.busName, mConnectionInfo.objectPath, CANONICAL_TELEPHONY_EMERGENCYMODE_IFACE);
        QDBusReply<QStringList> replyNumbers = connIface.call("EmergencyNumbers");
        if (replyNumbers.isValid()) {
            mEmergencyNumbers = replyNumbers.value();
            Q_EMIT emergencyNumbersChanged();
        }

        // connect the voicemail number changed signal
        dbusConnection.connect(mConnectionInfo.busName, mConnectionInfo.objectPath,
                               CANONICAL_TELEPHONY_VOICEMAIL_IFACE, "VoicemailNumberChanged",
                               this, SLOT(onVoicemailNumberChanged(QString)));

        QDBusInterface voicemailIface(mConnectionInfo.busName, mConnectionInfo.objectPath, CANONICAL_TELEPHONY_VOICEMAIL_IFACE);
        QDBusReply<QString> replyNumber = voicemailIface.call("VoicemailNumber");
        if (replyNumber.isValid()) {
            mVoicemailNumber = replyNumber.value();
            Q_EMIT voicemailNumberChanged();
        }

        watchSelfContactPresence();
    }

    Q_EMIT connectedChanged();
}

void AccountEntry::onEmergencyNumbersChanged(const QStringList &numbers)
{
    mEmergencyNumbers = numbers;
    Q_EMIT emergencyNumbersChanged();
}

void AccountEntry::onVoicemailNumberChanged(const QString &number)
{
    mVoicemailNumber = number;
    Q_EMIT voicemailNumberChanged();
}
