/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "telepathyhelper.h"

#include <TelepathyQt/AccountSet>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingAccount>

TelepathyHelper *TelepathyHelper::mTelepathyHelper = 0;

TelepathyHelper::TelepathyHelper(QObject *parent)
    : QObject(parent),
      mChannelHandler(0)
{
    if (!mTelepathyHelper) {
        mTelepathyHelper = this;
    }

    mChatManager = new ChatManager(this);

    mAccountFeatures << Tp::Account::FeatureCore;
    mContactFeatures << Tp::Contact::FeatureAlias
                     << Tp::Contact::FeatureCapabilities;
    mConnectionFeatures << Tp::Connection::FeatureCore
                        << Tp::Connection::FeatureSelfContact;

    mAccountManager = Tp::AccountManager::create(
            Tp::AccountFactory::create(QDBusConnection::sessionBus(), mAccountFeatures),
            Tp::ConnectionFactory::create(QDBusConnection::sessionBus(), mConnectionFeatures),
            Tp::ChannelFactory::create(QDBusConnection::sessionBus()),
            Tp::ContactFactory::create(mContactFeatures));

    connect(mAccountManager->becomeReady(Tp::AccountManager::FeatureCore),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));
}

TelepathyHelper::~TelepathyHelper()
{
}

TelepathyHelper *TelepathyHelper::instance()
{
    if (!mTelepathyHelper) {
        mTelepathyHelper = new TelepathyHelper();
    }

    return mTelepathyHelper;
}

ChatManager *TelepathyHelper::chatManager() const
{
    return mChatManager;
}

Tp::AccountPtr TelepathyHelper::account() const
{
    return mAccount;
}

void TelepathyHelper::initializeChannelHandler()
{
    mChannelHandler = new ChannelHandler();
    Tp::AbstractClientPtr handler(mChannelHandler);
    mClientRegistrar->registerClient(handler, "TelephonyApp");

    connect(mChannelHandler, SIGNAL(textChannelAvailable(Tp::TextChannelPtr)),
            mChatManager, SLOT(onTextChannelAvailable(Tp::TextChannelPtr)));

    channelHandlerCreated(mChannelHandler);
}

void TelepathyHelper::registerClients()
{
    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactoryPtr::constCast(mAccountManager->channelFactory());
    channelFactory->addCommonFeatures(Tp::Channel::FeatureCore);
    mClientRegistrar = Tp::ClientRegistrar::create(mAccountManager);
    initializeChannelHandler();
}

void TelepathyHelper::createAccount()
{
    QVariantMap props;
    props["org.freedesktop.Telepathy.Account.Icon"] = "im-ufa";
    connect(mAccountManager->createAccount("ufa", "ufa", "ufa", QVariantMap(), props),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountCreated(Tp::PendingOperation*)));
}

void TelepathyHelper::ensureAccountConnected()
{
    // if the account is not connected, request it to connect
    if (!mAccount->connection() || mAccount->connectionStatus() != Tp::ConnectionStatusConnected) {
        Tp::Presence presence(Tp::ConnectionPresenceTypeAvailable, "available", "online");
        mAccount->setRequestedPresence(presence);
    }
}

void TelepathyHelper::onAccountManagerReady(Tp::PendingOperation *op)
{
    Q_UNUSED(op)

    registerClients();

    Tp::AccountSetPtr accountSet = mAccountManager->accountsByProtocol("ufa");

    // if we have no ufa account, create one
    if (!accountSet->accounts().count()) {
        createAccount();
        return;
    }

    // in case we have two accounts, the first one to show on the list is going to be used
    mAccount = accountSet->accounts()[0];
    ensureAccountConnected();
}

void TelepathyHelper::onAccountCreated(Tp::PendingOperation *op)
{
    Tp::PendingAccount *pa = qobject_cast<Tp::PendingAccount*>(op);

    if (!pa)
        return;

    mAccount = pa->account();
    mAccount->setConnectsAutomatically(true);
    connect(mAccount->setEnabled(true),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountEnabled(Tp::PendingOperation*)));
}

void TelepathyHelper::onAccountEnabled(Tp::PendingOperation *op)
{
    // we might need to do more stuff once the account is enabled, but making sure it is connected is a good start
    ensureAccountConnected();
}
