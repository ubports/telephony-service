/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include <QtCore/QObject>
#include <TelepathyQt/PendingAccount>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/Account>
#include "telepathytest.h"
#include "telepathyhelper.h"
#include "accountentry.h"

TelepathyTest::TelepathyTest()
{

}

void TelepathyTest::initialize()
{
    Tp::registerTypes();

    // create an account manager instance to help testing
    Tp::Features accountFeatures;
    accountFeatures << Tp::Account::FeatureCore
                    << Tp::Account::FeatureProtocolInfo;
    Tp::Features contactFeatures;
    contactFeatures << Tp::Contact::FeatureAlias
                    << Tp::Contact::FeatureAvatarData
                    << Tp::Contact::FeatureAvatarToken
                    << Tp::Contact::FeatureCapabilities
                    << Tp::Contact::FeatureSimplePresence;
    Tp::Features connectionFeatures;
    connectionFeatures << Tp::Connection::FeatureCore
                       << Tp::Connection::FeatureSelfContact
                       << Tp::Connection::FeatureSimplePresence;

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
    channelFactory->addCommonFeatures(Tp::Channel::FeatureCore);

    mAccountManager = Tp::AccountManager::create(
            Tp::AccountFactory::create(QDBusConnection::sessionBus(), accountFeatures),
            Tp::ConnectionFactory::create(QDBusConnection::sessionBus(), connectionFeatures),
            channelFactory,
            Tp::ContactFactory::create(contactFeatures));

    mReady = false;
    connect(mAccountManager->becomeReady(Tp::AccountManager::FeatureCore),
            &Tp::PendingOperation::finished, [=]{
        Q_FOREACH(const Tp::AccountPtr &account, mAccountManager->allAccounts()) {
            Tp::PendingOperation *op = account->remove();
            WAIT_FOR(op->isFinished());
        }

        mReady = true;
    });

    TRY_VERIFY(mReady);
}

void TelepathyTest::doCleanup()
{
    // remove all accounts on every test to prevent garbage to go from one test to another
    Q_FOREACH(const Tp::AccountPtr &account, mAccounts) {
        QVERIFY(removeAccount(account));
    }
    QVERIFY(mAccounts.isEmpty());
}


Tp::AccountPtr TelepathyTest::addAccount(const QString &manager, const QString &protocol, const QString &displayName, const QVariantMap &parameters)
{
    bool finished = false;
    Tp::AccountPtr account;
    connect(mAccountManager->createAccount(manager, protocol, displayName, parameters), &Tp::PendingOperation::finished,
            [&](Tp::PendingOperation *op) {
        Tp::PendingAccount *pa = qobject_cast<Tp::PendingAccount*>(op);
        if (op->isError() || !pa) {
            qCritical() << "Failed to create account:" << op->errorName() << op->errorMessage();
            finished = true;
            return;
        }

        account = pa->account();
        // on the real device this is done by the telepathy-ofono plugin
        account->setEnabled(true);
        account->setConnectsAutomatically(true);
        finished = true;
    });

    WAIT_FOR(finished);
    WAIT_FOR(!account->connection().isNull());
    WAIT_FOR(account->connectionStatus() == Tp::ConnectionStatusConnected);
    WAIT_FOR(account->connection()->selfContact()->presence().type() == Tp::ConnectionPresenceTypeAvailable);

    mAccounts << account;
    return account;
}

bool TelepathyTest::removeAccount(const Tp::AccountPtr &account)
{
    bool success = false;
    bool finished = false;

    connect(account->remove(), &Tp::PendingOperation::finished,
            [&](Tp::PendingOperation *op) {
        success = !op->isError();
        finished = true;
    });

    WAIT_FOR(finished);

    if (success) {
        mAccounts.removeAll(account);
    }
    return success;
}


QList<Tp::AccountPtr> TelepathyTest::accounts() const
{
    return mAccounts;
}

void TelepathyTest::cleanup()
{
    doCleanup();
}
