/*
 * Copyright (C) 2013-2017 Canonical, Ltd.
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
#include "phoneutils.h"
#include "protocolmanager.h"
#include "telepathyhelper.h"

Q_DECLARE_METATYPE(Tp::ConnectionPtr);

namespace C {
#include <libintl.h>
}

AccountEntry::AccountEntry(const Tp::AccountPtr &account, QObject *parent) :
    QObject(parent), mAccount(account), mReady(false), mProtocol(0)
{
    qRegisterMetaType<Tp::ConnectionPtr>();
    initialize();
}

bool AccountEntry::ready() const
{
    return mReady;
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
    if (mAccount.isNull() || mAccount->connection().isNull() || mAccount->connection()->status() != Tp::ConnectionStatusConnected) {
        return false;
    }

    // we have to check if the account supports simple presence. In case it does, we use the self contact presence to determine
    // if this account is active.
    if (mAccount->connection()->hasInterface(TP_QT_IFACE_CONNECTION_INTERFACE_SIMPLE_PRESENCE)) {
        return (!mAccount->connection()->selfContact().isNull() &&
                mAccount->connection()->selfContact()->presence().type() != Tp::ConnectionPresenceTypeOffline);
    }

    // if it doesn't support simple presence, we  consider it online by having a connection in connected state
    return true;
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
           mAccount->connection()->status() == Tp::ConnectionStatusConnected;
}

AccountEntry::Capabilities AccountEntry::capabilities() const
{
    AccountEntry::Capabilities capabilities = CapabilityNone;

    if (!connected()) {
        return capabilities;
    }

    Tp::ConnectionCapabilities tpCapabilities = mAccount->capabilities();

    if (tpCapabilities.textChatrooms()) {
        capabilities |= (AccountEntry::Capabilities)AccountEntry::CapabilityTextChatrooms;
    }
    if (tpCapabilities.conferenceTextChats()) {
        capabilities |= (AccountEntry::Capabilities)AccountEntry::CapabilityConferenceTextChats;
    }
    if (tpCapabilities.conferenceTextChatsWithInvitees()) {
        capabilities |= (AccountEntry::Capabilities)AccountEntry::CapabilityConferenceTextChatsWithInvitees;
    }
    if (tpCapabilities.conferenceTextChatrooms()) {
        capabilities |= (AccountEntry::Capabilities)AccountEntry::CapabilityConferenceTextChatrooms;
    }
    if (tpCapabilities.conferenceTextChatroomsWithInvitees()) {
        capabilities |= (AccountEntry::Capabilities)AccountEntry::CapabilityConferenceTextChatroomsWithInvitees;
    }
    if (tpCapabilities.contactSearches()) {
        capabilities |= (AccountEntry::Capabilities)AccountEntry::CapabilityContactSearches;
    }

    return capabilities;
}

QVariantMap AccountEntry::accountProperties() const
{
    return mAccountProperties;
}

void AccountEntry::setAccountProperties(const QVariantMap &properties)
{
    TelepathyHelper::instance()->handlerInterface()->asyncCall("SetAccountProperties", mAccount->uniqueIdentifier(), properties);
    mAccountProperties = properties;
    Q_EMIT accountPropertiesChanged();
}

QVariantMap AccountEntry::parameters() const
{
    return mAccount->parameters();
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

bool AccountEntry::usePhoneNumbers() const
{
    return addressableVCardFields().contains("tel");
}

bool AccountEntry::compareIds(const QString &first, const QString &second) const
{
    // try the basic first
    if (first == second) {
        return true;
    }

    // if the account has "tel" in the addressable fields, also try phone compare
    if (addressableVCardFields().contains("tel")) {
        return PhoneUtils::comparePhoneNumbers(first, second) > PhoneUtils::NO_MATCH;
    }

    return false;
}

Protocol *AccountEntry::protocolInfo() const
{
    return mProtocol;
}

void AccountEntry::initialize()
{
    if (mAccount.isNull()) {
        return;
    }

    mProtocol = ProtocolManager::instance()->protocolByName(mAccount->protocolName());
    connect(ProtocolManager::instance(), &ProtocolManager::protocolsChanged, [=]() {
        mProtocol = ProtocolManager::instance()->protocolByName(mAccount->protocolName());
    });

    connect(this, &AccountEntry::addressableVCardFieldsChanged, &AccountEntry::usePhoneNumbersChanged);

    // propagate the display name changes
    connect(mAccount.data(),
            SIGNAL(removed()),
            SIGNAL(removed()));
 
    // propagate the display name changes
    connect(mAccount.data(),
            SIGNAL(displayNameChanged(QString)),
            SIGNAL(displayNameChanged()));

    connect(mAccount.data(),
            SIGNAL(connectionChanged(Tp::ConnectionPtr)),
            SLOT(onConnectionChanged(Tp::ConnectionPtr)));

    connect(mAccount.data(),
            SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)),
            SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)));

    connect(mAccount.data(), &Tp::Account::parametersChanged,
            this, &AccountEntry::parametersChanged);

    connect(this,
            SIGNAL(connectedChanged()),
            SIGNAL(activeChanged()));

    Q_EMIT accountIdChanged();
    
    // we have to postpone this call to give telepathyhelper time to connect the signals
    QMetaObject::invokeMethod(this, "onConnectionChanged", Qt::QueuedConnection, Q_ARG(Tp::ConnectionPtr, mAccount->connection()));
    QMetaObject::invokeMethod(this, "accountReady", Qt::QueuedConnection);

    // FIXME: change it to be asynchronous
    if (QCoreApplication::applicationName() != "telephony-service-handler") {
        QDBusReply<QVariantMap> reply = TelepathyHelper::instance()->handlerInterface()->call("GetAccountProperties", mAccount->uniqueIdentifier());

        if (!reply.isValid()) {
            return;
        }
        mAccountProperties = reply;
        Q_EMIT accountPropertiesChanged();
    }
    mReady = true;
}

void AccountEntry::watchSelfContactPresence()
{
    if (mAccount.isNull() || mAccount->connection().isNull() || mAccount->connection()->selfContact().isNull()) {
        return;
    }

    connect(mAccount->connection()->selfContact().data(),
            SIGNAL(presenceChanged(Tp::Presence)),
            SIGNAL(statusChanged()));

    connect(mAccount->connection()->selfContact().data(),
            SIGNAL(presenceChanged(Tp::Presence)),
            SIGNAL(statusMessageChanged()));

    connect(mAccount->connection()->selfContact().data(),
            SIGNAL(presenceChanged(Tp::Presence)),
            SIGNAL(activeChanged()));

    connect(mAccount->connection()->selfContact().data(),
            SIGNAL(presenceChanged(Tp::Presence)),
            SIGNAL(connectedChanged()));

}

void AccountEntry::onSelfContactChanged()
{
    watchSelfContactPresence();

    Q_EMIT connectedChanged();
    Q_EMIT selfContactIdChanged();
}

void AccountEntry::onConnectionChanged(Tp::ConnectionPtr connection)
{
    if (!connection.isNull()) {
        mConnectionInfo.busName = connection->busName();
        mConnectionInfo.objectPath = connection->objectPath();

        connect(connection.data(),
                SIGNAL(selfContactChanged()),
                SLOT(onSelfContactChanged()));

        watchSelfContactPresence();
    } else {
        mConnectionInfo.busName = QString();
        mConnectionInfo.objectPath = QString();
    }

    Q_EMIT connectedChanged();
    Q_EMIT selfContactIdChanged();
    Q_EMIT capabilitiesChanged();
}

void AccountEntry::addAccountLabel(const QString &accountId, QString &text)
{
    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
    if (account && account->type() == AccountEntry::PhoneAccount &&
            TelepathyHelper::instance()->multiplePhoneAccounts()) {
        text += QString(" - [%1]").arg(account->displayName());
    }
}
