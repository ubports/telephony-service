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

#include "ofonoaccountentry.h"
#include "phoneutils.h"
#include "telepathyhelper.h"

OfonoAccountEntry::OfonoAccountEntry(const Tp::AccountPtr &account, QObject *parent) :
    AccountEntry(account, parent), mVoicemailCount(0), mVoicemailIndicator(false)
{
    // the sim lock detection is based on the status message, so whenever it
    // changes, it might mean the sim lock state changed too
    connect(this,
            SIGNAL(statusChanged()),
            SIGNAL(simLockedChanged()));
    connect(this,
            SIGNAL(statusMessageChanged()),
            SIGNAL(networkNameChanged()));
    connect(this,
            SIGNAL(statusMessageChanged()),
            SIGNAL(emergencyCallsAvailableChanged()));

    mUssdManager = new USSDManager(this, this);
}

USSDManager *OfonoAccountEntry::ussdManager() const
{
    return mUssdManager;
}

QStringList OfonoAccountEntry::emergencyNumbers() const
{
    return mEmergencyNumbers;
}

QString OfonoAccountEntry::voicemailNumber() const
{
    return mVoicemailNumber;
}

uint OfonoAccountEntry::voicemailCount() const
{
    return mVoicemailCount;
}

bool OfonoAccountEntry::voicemailIndicator() const
{
    return mVoicemailIndicator;
}

QString OfonoAccountEntry::networkName() const
{
    // FIXME: maybe it is safer to reimplement here, but for ofono accounts the status message really is the
    // network name
    return statusMessage();
}

bool OfonoAccountEntry::emergencyCallsAvailable() const
{
    if (mAccount.isNull() || mAccount->connection().isNull() || mAccount->connection()->selfContact().isNull()) {
        return false;
    }

    QString status = mAccount->connection()->selfContact()->presence().status();
    return status != "flightmode" && status != "nomodem" && status != "";
}

bool OfonoAccountEntry::simLocked() const
{
    if (mAccount.isNull() || mAccount->connection().isNull() || mAccount->connection()->selfContact().isNull()) {
        return false;
    }
    Tp::Presence presence = mAccount->connection()->selfContact()->presence();
    return (presence.type() == Tp::ConnectionPresenceTypeAway && presence.status() == "simlocked");
}

QString OfonoAccountEntry::serial() const
{
    return mSerial;
}

AccountEntry::AccountType OfonoAccountEntry::type() const
{
    return AccountEntry::PhoneAccount;
}

bool OfonoAccountEntry::connected() const
{
    return !mAccount.isNull() && !mAccount->connection().isNull() &&
           !mAccount->connection()->selfContact().isNull() &&
            mAccount->connection()->selfContact()->presence().type() == Tp::ConnectionPresenceTypeAvailable;
}

bool OfonoAccountEntry::compareIds(const QString &first, const QString &second) const
{
    return PhoneUtils::comparePhoneNumbers(first, second);
}

QStringList OfonoAccountEntry::addressableVCardFields()
{
    return mAccount->protocolInfo().addressableVCardFields();
}

void OfonoAccountEntry::onEmergencyNumbersChanged(const QStringList &numbers)
{
    mEmergencyNumbers = numbers;
    Q_EMIT emergencyNumbersChanged();
}

void OfonoAccountEntry::onVoicemailNumberChanged(const QString &number)
{
    mVoicemailNumber = number;
    Q_EMIT voicemailNumberChanged();
}

void OfonoAccountEntry::onVoicemailCountChanged(uint count)
{
    mVoicemailCount = count;
    Q_EMIT voicemailCountChanged();
}

void OfonoAccountEntry::onVoicemailIndicatorChanged(bool visible)
{
    mVoicemailIndicator = visible;
    Q_EMIT voicemailIndicatorChanged();
}

void OfonoAccountEntry::onConnectionChanged()
{
    // make sure the generic code is also run
    AccountEntry::onConnectionChanged();

    QDBusConnection dbusConnection = QDBusConnection::sessionBus();

    if (!mAccount->connection()) {
        // disconnect any previous dbus connections
        if (!mConnectionInfo.objectPath.isEmpty()) {
            dbusConnection.disconnect(mConnectionInfo.busName, mConnectionInfo.objectPath,
                                      CANONICAL_TELEPHONY_EMERGENCYMODE_IFACE, "EmergencyNumbersChanged",
                                      this, SLOT(onEmergencyNumbersChanged(QStringList)));
        }
    } else {
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
        } else {
            qWarning() << "Could not get voicemail number!";
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

        // and get the serial
        QDBusInterface ussdIface(mConnectionInfo.busName, mConnectionInfo.objectPath, CANONICAL_TELEPHONY_USSD_IFACE);
        mSerial = ussdIface.property("Serial").toString();
    }

    Q_EMIT serialChanged();
}
