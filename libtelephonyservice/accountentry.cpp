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

QString AccountEntry::networkName() const
{
    if (mAccount.isNull() || mAccount->connection().isNull() || mAccount->connection()->selfContact().isNull()) {
        return QString::null;
    }
    Tp::Presence presence = mAccount->connection()->selfContact()->presence();
    if (presence.type() == Tp::ConnectionPresenceTypeAvailable) {
        return presence.statusMessage();
    }
    return QString::null;
}

bool AccountEntry::simLocked() const
{
    if (mAccount.isNull() || mAccount->connection().isNull() || mAccount->connection()->selfContact().isNull()) {
        return false;
    }
    Tp::Presence presence = mAccount->connection()->selfContact()->presence();
    return (presence.type() == Tp::ConnectionPresenceTypeAway && presence.status() == "simlocked");
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

QStringList AccountEntry::emergencyNumbers() const
{
    return mEmergencyNumbers;
}

QString AccountEntry::voicemailNumber() const
{
    return mVoicemailNumber;
}

bool AccountEntry::voicemailIndicator() const
{
    return mVoicemailIndicator;
}

uint AccountEntry::voicemailCount() const
{
    return mVoicemailCount;
}

Tp::AccountPtr AccountEntry::account() const
{
    return mAccount;
}

bool AccountEntry::emergencyCallsAvailable() const
{
    if (mAccount.isNull() || mAccount->connection().isNull() || mAccount->connection()->selfContact().isNull()) {
        return false;
    }

    QString status = mAccount->connection()->selfContact()->presence().status();
    return status != "flightmode" && status != "nomodem" && status != "";
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
    connect(mAccount->connection()->selfContact().data(),
            SIGNAL(presenceChanged(Tp::Presence)),
            SIGNAL(networkNameChanged()));
    connect(mAccount->connection()->selfContact().data(),
            SIGNAL(presenceChanged(Tp::Presence)),
            SIGNAL(emergencyCallsAvailableChanged()));
    connect(mAccount->connection()->selfContact().data(),
            SIGNAL(presenceChanged(Tp::Presence)),
            SIGNAL(simLockedChanged()));
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

        // connect the voicemail count changed signal
        dbusConnection.connect(mConnectionInfo.busName, mConnectionInfo.objectPath,
                               CANONICAL_TELEPHONY_VOICEMAIL_IFACE, "VoicemailCountChanged",
                               this, SLOT(onVoicemailCountChanged(uint)));

        QDBusReply<uint> replyCount = voicemailIface.call("VoicemailCount");
        if (replyCount.isValid()) {
            mVoicemailCount = replyCount.value();
            Q_EMIT voicemailCountChanged();
        }

        // connect the voicemail indicator changed signal
        dbusConnection.connect(mConnectionInfo.busName, mConnectionInfo.objectPath,
                               CANONICAL_TELEPHONY_VOICEMAIL_IFACE, "VoicemailIndicatorChanged",
                               this, SLOT(onVoicemailIndicatorChanged(bool)));

        QDBusReply<bool> replyIndicator = voicemailIface.call("VoicemailIndicator");
        if (replyIndicator.isValid()) {
            mVoicemailIndicator = replyIndicator.value();
            Q_EMIT voicemailIndicatorChanged();
        }

        watchSelfContactPresence();
    }

    Q_EMIT networkNameChanged();
    Q_EMIT connectedChanged();
    Q_EMIT simLockedChanged();
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

void AccountEntry::onVoicemailCountChanged(uint count)
{
    mVoicemailCount = count;
    Q_EMIT voicemailCountChanged();
}

void AccountEntry::onVoicemailIndicatorChanged(bool visible)
{
    mVoicemailIndicator = visible;
    Q_EMIT voicemailIndicatorChanged();
}
