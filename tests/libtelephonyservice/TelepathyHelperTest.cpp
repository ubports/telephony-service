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
#include <QtTest/QtTest>
#include <TelepathyQt/PendingAccount>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/Account>
#include "accountentry.h"
#include "accountentryfactory.h"
#include "telepathyhelper.h"
#include "mockcontroller.h"

#define DEFAULT_TIMEOUT 15000

class TelepathyHelperTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testConnected();
    void testAccounts();
    void testAccountSorting();
    void testAccountIds();
    void testActiveAccounts();
    void testAccountForId();
    void testAccountForConnection();
    void testEmergencyCallsAvailable();

    // helper slots
    void onAccountManagerReady(Tp::PendingOperation *op);
    Tp::AccountPtr addAccount(const QString &manager,
                              const QString &protocol,
                              const QString &displayName,
                              const QVariantMap &parameters = QVariantMap());
    bool removeAccount(const Tp::AccountPtr &account);

private:
    MockController *mGenericController;
    MockController *mPhoneController;
    Tp::AccountManagerPtr mAccountManager;

    bool ready;
};

void TelepathyHelperTest::initTestCase()
{
    Tp::registerTypes();

    QSignalSpy spy(TelepathyHelper::instance(), SIGNAL(setupReady()));
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, DEFAULT_TIMEOUT);
    QTRY_VERIFY_WITH_TIMEOUT(TelepathyHelper::instance()->connected(), DEFAULT_TIMEOUT);

    // create an account manager instance to help testing
    Tp::Features accountFeatures;
    accountFeatures << Tp::Account::FeatureCore;
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

    ready = false;
    connect(mAccountManager->becomeReady(Tp::AccountManager::FeatureCore),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));

    QTRY_VERIFY(ready);

    // and create the mock controller
    mGenericController = new MockController("mock", this);
    mPhoneController = new MockController("ofono", this);
}

void TelepathyHelperTest::testConnected()
{
    QSignalSpy connectedChangedSpy(TelepathyHelper::instance(), SIGNAL(connectedChanged()));

    // check that the default status is connected (always true for mock accounts)
    QVERIFY(TelepathyHelper::instance()->connected());

    // set one of the accounts offline and check that the connected status stays true
    mGenericController->setOnline(false);
    QTest::qWait(1000);
    QCOMPARE(connectedChangedSpy.count(), 0);
    QVERIFY(TelepathyHelper::instance()->connected());

    // and set the other account as offline too. This time connected needs to change to false
    mPhoneController->setOnline(false);
    QTRY_COMPARE(connectedChangedSpy.count(), 1);
    QVERIFY(!TelepathyHelper::instance()->connected());

    // now set one of the accounts back online
    connectedChangedSpy.clear();
    mPhoneController->setOnline(true);
    QTRY_COMPARE(connectedChangedSpy.count(), 1);
    QVERIFY(TelepathyHelper::instance()->connected());

    // and the other one just in case
    connectedChangedSpy.clear();
    mGenericController->setOnline(true);
    QTest::qWait(1000);
    QCOMPARE(connectedChangedSpy.count(), 0);
    QVERIFY(TelepathyHelper::instance()->connected());
}

void TelepathyHelperTest::testAccounts()
{
    QCOMPARE(TelepathyHelper::instance()->accounts().count(), 2);
    AccountEntry *first = TelepathyHelper::instance()->accounts()[0];
    AccountEntry *second = TelepathyHelper::instance()->accounts()[1];
    QVERIFY(first->accountId() != second->accountId());

    // now check that new accounts are captured
    QSignalSpy accountsChangedSpy(TelepathyHelper::instance(), SIGNAL(accountsChanged()));
    Tp::AccountPtr newAccount = addAccount("mock", "mock", "extra");
    QVERIFY(!newAccount.isNull());

    QTRY_COMPARE(accountsChangedSpy.count(), 1);
    QCOMPARE(TelepathyHelper::instance()->accounts().count(), 3);

    bool accountFound = false;
    Q_FOREACH(AccountEntry *entry, TelepathyHelper::instance()->accounts()) {
        if (entry->accountId() == newAccount->uniqueIdentifier()) {
            accountFound = true;
            break;
        }
    }
    QVERIFY(accountFound);

    // now remove the extra account and make sure it is properly removed
    accountsChangedSpy.clear();
    QVERIFY(removeAccount(newAccount));
    QTRY_COMPARE(accountsChangedSpy.count(), 1);
    QCOMPARE(TelepathyHelper::instance()->accounts().count(), 2);
    QCOMPARE(TelepathyHelper::instance()->accounts()[0]->accountId(), first->accountId());
    QCOMPARE(TelepathyHelper::instance()->accounts()[1]->accountId(), second->accountId());
}

void TelepathyHelperTest::testAccountSorting()
{
    // create two accounts with modem-objpath parameters and make sure they are listed first
    QVariantMap parameters;
    parameters["modem-objpath"] = "/phonesim1";
    Tp::AccountPtr firstAccount = addAccount("ofono", "ofono", "firstPhoneAccount", parameters);
    QVERIFY(!firstAccount.isNull());

    parameters["modem-objpath"] = "/phonesim2";
    Tp::AccountPtr secondAccount = addAccount("ofono", "ofono", "secondPhoneAccount", parameters);
    QVERIFY(!secondAccount.isNull());

    // wait for the accounts to appear;
    QTRY_COMPARE(TelepathyHelper::instance()->accounts().count(), 4);

    // and check the order
    QCOMPARE(TelepathyHelper::instance()->accounts()[0]->accountId(), firstAccount->uniqueIdentifier());
    QCOMPARE(TelepathyHelper::instance()->accounts()[1]->accountId(), secondAccount->uniqueIdentifier());

    // now add a third account that should go before the two others
    parameters["modem-objpath"] = "/phonesim0";
    Tp::AccountPtr thirdAccount = addAccount("ofono", "ofono", "thirdPhoneAccount", parameters);
    QVERIFY(!thirdAccount.isNull());

    // wait for the accounts to appear;
    QTRY_COMPARE(TelepathyHelper::instance()->accounts().count(), 5);
    QCOMPARE(TelepathyHelper::instance()->accounts()[0]->accountId(), thirdAccount->uniqueIdentifier());

    // and remove the created accounts
    QVERIFY(removeAccount(firstAccount));
    QVERIFY(removeAccount(secondAccount));
    QVERIFY(removeAccount(thirdAccount));
}

void TelepathyHelperTest::testAccountIds()
{
    QCOMPARE(TelepathyHelper::instance()->accountIds().count(), 2);
    QVERIFY(TelepathyHelper::instance()->accountIds().contains("mock/mock/account0"));
    QVERIFY(TelepathyHelper::instance()->accountIds().contains("mock/ofono/account0"));

    // now check that new accounts are captured
    QSignalSpy accountIdsChangedSpy(TelepathyHelper::instance(), SIGNAL(accountIdsChanged()));
    Tp::AccountPtr newAccount = addAccount("mock", "mock", "extra");
    QVERIFY(!newAccount.isNull());

    QTRY_COMPARE(accountIdsChangedSpy.count(), 1);
    QCOMPARE(TelepathyHelper::instance()->accountIds().count(), 3);

    // just to make sure check that each account id matches one account
    for (int i = 0; i < 3; ++i) {
        QCOMPARE(TelepathyHelper::instance()->accountIds()[i], TelepathyHelper::instance()->accounts()[i]->accountId());
    }

    // now remove the extra account and make sure it is properly removed
    accountIdsChangedSpy.clear();
    QVERIFY(removeAccount(newAccount));
    QTRY_COMPARE(accountIdsChangedSpy.count(), 1);
    QCOMPARE(TelepathyHelper::instance()->accountIds().count(), 2);
    QCOMPARE(TelepathyHelper::instance()->accountIds()[0], TelepathyHelper::instance()->accounts()[0]->accountId());
    QCOMPARE(TelepathyHelper::instance()->accountIds()[1], TelepathyHelper::instance()->accounts()[1]->accountId());
}

void TelepathyHelperTest::testActiveAccounts()
{
    QSignalSpy activeAccountsSpy(TelepathyHelper::instance(), SIGNAL(activeAccountsChanged()));

    // at startup, all accounts are active, so make sure we got two active accounts
    QCOMPARE(TelepathyHelper::instance()->activeAccounts().count(), 2);

    // now set one of the accounts as offline and make sure it is captured
    mGenericController->setOnline(false);
    QTRY_COMPARE(activeAccountsSpy.count(), 1);
    QCOMPARE(TelepathyHelper::instance()->activeAccounts().count(), 1);
    QCOMPARE(TelepathyHelper::instance()->activeAccounts()[0]->accountId(), QString("mock/ofono/account0"));

    // set the other account offline to make sure
    activeAccountsSpy.clear();
    mPhoneController->setOnline(false);
    QTRY_COMPARE(activeAccountsSpy.count(), 1);
    QVERIFY(TelepathyHelper::instance()->activeAccounts().isEmpty());

    // and set both accounts online again
    activeAccountsSpy.clear();
    mGenericController->setOnline(true);
    mPhoneController->setOnline(true);
    QTRY_COMPARE(activeAccountsSpy.count(), 2);
    QCOMPARE(TelepathyHelper::instance()->activeAccounts().count(), 2);
}

void TelepathyHelperTest::testAccountForId()
{
    AccountEntry *genericAccount = TelepathyHelper::instance()->accountForId("mock/mock/account0");
    QVERIFY(genericAccount);
    QCOMPARE(genericAccount->accountId(), QString("mock/mock/account0"));

    AccountEntry *phoneAccount = TelepathyHelper::instance()->accountForId("mock/ofono/account0");
    QVERIFY(phoneAccount);
    QCOMPARE(phoneAccount->accountId(), QString("mock/ofono/account0"));
}

void TelepathyHelperTest::testAccountForConnection()
{
    Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->accounts()) {
        AccountEntry *entry = TelepathyHelper::instance()->accountForConnection(account->account()->connection());
        QVERIFY(entry);
        QCOMPARE(entry, account);
    }
}

void TelepathyHelperTest::testEmergencyCallsAvailable()
{
    QSignalSpy emergencyCallsSpy(TelepathyHelper::instance(), SIGNAL(emergencyCallsAvailableChanged()));

    // check that calls are available by default
    QVERIFY(TelepathyHelper::instance()->emergencyCallsAvailable());

    // set the generic account as "flightmode" and make sure it doesn't affect the emergencyCallsAvailable
    mGenericController->setPresence("flightmode", "");
    QTest::qWait(500);
    QCOMPARE(emergencyCallsSpy.count(), 0);
    QVERIFY(TelepathyHelper::instance()->emergencyCallsAvailable());

    // now set the phone account as "flightmode", and see if the emergencyCallsAvailable value
    mPhoneController->setPresence("flightmode", "");
    QTRY_COMPARE(emergencyCallsSpy.count(), 1);
    QVERIFY(!TelepathyHelper::instance()->emergencyCallsAvailable());

    // set the generic account online and check if it affects the value
    emergencyCallsSpy.clear();
    mGenericController->setOnline(true);
    QTest::qWait(500);
    QCOMPARE(emergencyCallsSpy.count(), 0);
    QVERIFY(!TelepathyHelper::instance()->emergencyCallsAvailable());

    // and finally set the phone account back online
    mPhoneController->setOnline(true);
    QTRY_COMPARE(emergencyCallsSpy.count(), 1);
    QVERIFY(TelepathyHelper::instance()->emergencyCallsAvailable());
}

// -------------- Helpers -----------------------

void TelepathyHelperTest::onAccountManagerReady(Tp::PendingOperation *op)
{
    ready = true;
}

Tp::AccountPtr TelepathyHelperTest::addAccount(const QString &manager, const QString &protocol, const QString &displayName, const QVariantMap &parameters)
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
        finished = true;
    });

    while (!finished) {
        QTest::qWait(100);
    }
    return account;
}

bool TelepathyHelperTest::removeAccount(const Tp::AccountPtr &account)
{
    bool success = false;
    bool finished = false;

    connect(account->remove(), &Tp::PendingOperation::finished,
            [&](Tp::PendingOperation *op) {
        success = !op->isError();
        finished = true;
    });

    while (!finished) {
        QTest::qWait(100);
    }
    return success;
}

QTEST_MAIN(TelepathyHelperTest)
#include "TelepathyHelperTest.moc"
