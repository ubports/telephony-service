/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "telepathyhelper.h"
#include "accountentry.h"
#include "chatmanager.h"
#include "callmanager.h"
#include "config.h"
#include "greetercontacts.h"

#include <TelepathyQt/AccountSet>
#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingAccount>

TelepathyHelper::TelepathyHelper(QObject *parent)
    : QObject(parent),
      mChannelObserver(0),
      mFirstTime(true),
      mConnected(false),
      mHandlerInterface(0)
{
    mAccountFeatures << Tp::Account::FeatureCore;
    mContactFeatures << Tp::Contact::FeatureAlias
                     << Tp::Contact::FeatureAvatarData
                     << Tp::Contact::FeatureAvatarToken
                     << Tp::Contact::FeatureCapabilities
                     << Tp::Contact::FeatureSimplePresence;
    mConnectionFeatures << Tp::Connection::FeatureCore
                        << Tp::Connection::FeatureSelfContact
                        << Tp::Connection::FeatureSimplePresence;

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
    channelFactory->addCommonFeatures(Tp::Channel::FeatureCore);

    // FIXME: at some point this needs to be fixed in tp-qt itself.
    channelFactory->setSubclassFor<Tp::CallChannel>(audioConferenceSpec());

    mAccountManager = Tp::AccountManager::create(
            Tp::AccountFactory::create(QDBusConnection::sessionBus(), mAccountFeatures),
            Tp::ConnectionFactory::create(QDBusConnection::sessionBus(), mConnectionFeatures),
            channelFactory,
            Tp::ContactFactory::create(mContactFeatures));

    connect(mAccountManager->becomeReady(Tp::AccountManager::FeatureCore),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));

    mClientRegistrar = Tp::ClientRegistrar::create(mAccountManager);
    connect(this, SIGNAL(accountReady()), SIGNAL(setupReady()));
}

TelepathyHelper::~TelepathyHelper()
{
}

TelepathyHelper *TelepathyHelper::instance()
{
    static TelepathyHelper* helper = new TelepathyHelper();
    return helper;
}

QStringList TelepathyHelper::accountIds()
{
    QStringList ids;

    if (QCoreApplication::applicationName() == "telephony-service-handler" || mAccounts.size() != 0) {
        Q_FOREACH(const AccountEntry *account, mAccounts) {
            ids << account->accountId();
        }
    } else if (!GreeterContacts::instance()->isGreeterMode()) {
        // if we are in greeter mode, we should not initialize the handler to get the account IDs
        QDBusReply<QStringList> reply = handlerInterface()->call("AccountIds");
        if (reply.isValid()) {
            ids = reply.value();
        }
    }

    return ids;
}

QList<AccountEntry*> TelepathyHelper::accounts() const
{
    return mAccounts;
}

QQmlListProperty<AccountEntry> TelepathyHelper::qmlAccounts()
{
    return QQmlListProperty<AccountEntry>(this, 0, accountsCount, accountAt);
}

ChannelObserver *TelepathyHelper::channelObserver() const
{
    return mChannelObserver;
}

QDBusInterface *TelepathyHelper::handlerInterface() const
{
    // delay the loading of the handler interface, as it seems this is triggering
    // the dbus activation of the handler process
    if (!mHandlerInterface) {
        mHandlerInterface = new QDBusInterface("com.canonical.TelephonyServiceHandler",
                                               "/com/canonical/TelephonyServiceHandler",
                                               "com.canonical.TelephonyServiceHandler",
                                               QDBusConnection::sessionBus(),
                                               const_cast<TelepathyHelper*>(this));
    }
    return mHandlerInterface;
}

bool TelepathyHelper::connected() const
{
    if (QCoreApplication::applicationName() != "telephony-service-handler" &&
        mAccounts.isEmpty() &&
        !GreeterContacts::instance()->isGreeterMode()) {
        // get the status from the handler
        QDBusReply<bool> reply = handlerInterface()->call("IsConnected");
        if (reply.isValid()) {
            return reply.value();
        }
    }

    return mConnected;
}


void TelepathyHelper::registerChannelObserver(const QString &observerName)
{
    QString name = observerName;

    if (name.isEmpty()) {
        name = "TelephonyPluginObserver";
    }

    if (mChannelObserver) {
        mChannelObserver->deleteLater();
    }

    mChannelObserver = new ChannelObserver(this);
    registerClient(mChannelObserver, name);

    // messages
    connect(mChannelObserver, SIGNAL(textChannelAvailable(Tp::TextChannelPtr)),
            ChatManager::instance(), SLOT(onTextChannelAvailable(Tp::TextChannelPtr)));

    // calls
    connect(mChannelObserver, SIGNAL(callChannelAvailable(Tp::CallChannelPtr)),
            CallManager::instance(), SLOT(onCallChannelAvailable(Tp::CallChannelPtr)));

    Q_EMIT channelObserverCreated(mChannelObserver);
}

void TelepathyHelper::unregisterChannelObserver()
{
    Tp::AbstractClientPtr clientPtr(mChannelObserver);
    if (clientPtr) {
        mClientRegistrar->unregisterClient(clientPtr);
    }
    mChannelObserver->deleteLater();
    mChannelObserver = NULL;
    Q_EMIT channelObserverUnregistered();
}

QStringList TelepathyHelper::supportedProtocols() const
{
    QStringList protocols;
    protocols << "ufa"
              << "tel"
              << "ofono"
              << "mock"; // used for tests
    return protocols;
}

void TelepathyHelper::setupAccountEntry(AccountEntry *entry)
{
    connect(entry,
            SIGNAL(connectedChanged()),
            SLOT(updateConnectedStatus()));
    connect(entry,
            SIGNAL(accountReady()),
            SLOT(onAccountReady()));
}

void TelepathyHelper::registerClient(Tp::AbstractClient *client, QString name)
{
    Tp::AbstractClientPtr clientPtr(client);
    bool succeeded = mClientRegistrar->registerClient(clientPtr, name);
    if (!succeeded) {
        name.append("%1");
        int count = 0;
        // limit the number of registered clients to 20, that should be a safe margin
        while (!succeeded && count < 20) {
            succeeded = mClientRegistrar->registerClient(clientPtr, name.arg(++count));
            if (succeeded) {
                name = name.arg(count);
            }
        }
    }

    if (succeeded) {
        QObject *object = dynamic_cast<QObject*>(client);
        if (object) {
            object->setProperty("clientName", TP_QT_IFACE_CLIENT + "." + name );
        }
    }
}

AccountEntry *TelepathyHelper::accountForConnection(const Tp::ConnectionPtr &connection) const
{
    if (connection.isNull()) {
        return 0;
    }

    Q_FOREACH(AccountEntry *accountEntry, mAccounts) {
        if (accountEntry->account()->connection() == connection) {
            return accountEntry;
        }
    }

    return 0;
}

AccountEntry *TelepathyHelper::accountForId(const QString &accountId) const
{
    Q_FOREACH(AccountEntry *account, mAccounts) {
        if (account->accountId() == accountId) {
            return account;
        }
    }

    return 0;
}

Tp::ChannelClassSpec TelepathyHelper::audioConferenceSpec()
{
    static Tp::ChannelClassSpec spec;
    if (!spec.isValid()) {
        spec = Tp::ChannelClassSpec(TP_QT_IFACE_CHANNEL_TYPE_CALL, Tp::HandleTypeNone);
        spec.setCallInitialAudioFlag();
    }
    return spec;
}

int TelepathyHelper::accountsCount(QQmlListProperty<AccountEntry> *p)
{
    Q_UNUSED(p)
    return TelepathyHelper::instance()->accounts().count();
}

AccountEntry *TelepathyHelper::accountAt(QQmlListProperty<AccountEntry> *p, int index)
{
    Q_UNUSED(p)
    return TelepathyHelper::instance()->accounts()[index];
}

void TelepathyHelper::onAccountManagerReady(Tp::PendingOperation *op)
{
    Q_UNUSED(op)

    Tp::AccountSetPtr accountSet;
    // try to find an account of the one of supported protocols
    Q_FOREACH(const QString &protocol, supportedProtocols()) {
        accountSet = mAccountManager->accountsByProtocol(protocol);
        Q_FOREACH(const Tp::AccountPtr &account, accountSet->accounts()) {
            AccountEntry *accountEntry = new AccountEntry(account, this);
            setupAccountEntry(accountEntry);
            mAccounts << accountEntry;
        }
    }

    if (mAccounts.count() == 0) {
        Q_EMIT setupReady();
        return;
    }

    // FIXME: handle dynamic account adding and removing

    Q_EMIT accountIdsChanged();
    Q_EMIT accountsChanged();
}

void TelepathyHelper::onAccountReady()
{
    if (mFirstTime) {
        Q_EMIT accountReady();
    }

    mFirstTime = false;
}

void TelepathyHelper::updateConnectedStatus()
{
    bool previousConnectedStatus = mConnected;
    mConnected = false;

    // check if any of the accounts is currently connected
    Q_FOREACH(AccountEntry *account, mAccounts) {
        if (account->connected()) {
            mConnected = true;
            break;
        }
    }

    // avoid emitting changed signals when the pro
    if (mConnected != previousConnectedStatus) {
        Q_EMIT connectedChanged();
    }
}
