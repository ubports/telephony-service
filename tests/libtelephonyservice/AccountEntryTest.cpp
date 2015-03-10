/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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
#include "accountentry.h"
#include "accountentryfactory.h"
#include "telepathyhelper.h"
#include "mockcontroller.h"

#define DEFAULT_TIMEOUT 15000

class AccountEntryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testAccountId();
    void testActive();
    void testDisplayName();
    void testStatus();
    void testStatusMessage();
    void testConnected();
    void testCompareIds_data();
    void testCompareIds();

private:
    AccountEntry *mAccount;
    Tp::AccountPtr mTpAccount;
    AccountEntry *mNullAccount;
    MockController *mMockController;
};

void AccountEntryTest::initTestCase()
{
    Tp::registerTypes();

    QSignalSpy spy(TelepathyHelper::instance(), SIGNAL(setupReady()));
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, DEFAULT_TIMEOUT);
    QTRY_VERIFY_WITH_TIMEOUT(TelepathyHelper::instance()->connected(), DEFAULT_TIMEOUT);

    mAccount = TelepathyHelper::instance()->accountForId("mock/mock/account0");
    QVERIFY(mAccount);

    mTpAccount = mAccount->account();
    QVERIFY(!mTpAccount.isNull());
    QTRY_VERIFY(mTpAccount->isReady(Tp::Account::FeatureCore));

    // wait for the connection to appear
    QTRY_VERIFY(!mTpAccount->connection().isNull());

    // create a null account
    mNullAccount = AccountEntryFactory::createEntry(Tp::AccountPtr(), this);

    // and create the mock controller
    mMockController = new MockController("mock", this);
}

void AccountEntryTest::testAccountId()
{
    QCOMPARE(mAccount->accountId(), mTpAccount->uniqueIdentifier());
    QVERIFY(mNullAccount->accountId().isNull());
}

void AccountEntryTest::testActive()
{
    QSignalSpy activeChangedSpy(mAccount, SIGNAL(activeChanged()));

    // the mock account is enabled/connected by default, so make sure it is like that
    QVERIFY(mAccount->active());

    // now set the account offline and see if the active flag changes correctly
    mMockController->setOnline(false);
    QTRY_VERIFY(!mAccount->active());
    QCOMPARE(activeChangedSpy.count(), 1);

    // now re-enable the account and check that the entry is updated
    activeChangedSpy.clear();
    mMockController->setOnline(true);
    QTRY_VERIFY(mAccount->active());
    QCOMPARE(activeChangedSpy.count(), 1);

    // check that for a null account active is false
    QVERIFY(!mNullAccount->active());
}

void AccountEntryTest::testDisplayName()
{
    QSignalSpy displayNameChangedSpy(mAccount, SIGNAL(displayNameChanged()));

    // check that the value is correct already
    QCOMPARE(mAccount->displayName(), mTpAccount->displayName());

    // now try to set the display in the telepathy account directly and see that the entry gets updated
    QString newDisplayName = "some other display name";
    mTpAccount->setDisplayName(newDisplayName);
    QTRY_COMPARE(mAccount->displayName(), newDisplayName);
    QCOMPARE(displayNameChangedSpy.count(), 1);

    // and try setting the display name in the entry itself
    displayNameChangedSpy.clear();
    newDisplayName = "changing again";
    mAccount->setDisplayName(newDisplayName);
    QTRY_COMPARE(mAccount->displayName(), newDisplayName);
    QCOMPARE(displayNameChangedSpy.count(), 1);
    QCOMPARE(mTpAccount->displayName(), newDisplayName);

    // check that for a null account the displayName is null
    QVERIFY(mNullAccount->displayName().isNull());
}

void AccountEntryTest::testStatus()
{
    QSignalSpy statusChangedSpy(mAccount, SIGNAL(statusChanged()));

    // check that the value is correct already
    QCOMPARE(mAccount->status(), mTpAccount->connection()->selfContact()->presence().status());

    // and now set a new value
    Tp::Presence presence(Tp::ConnectionPresenceTypeAway, "away", "away");
    mTpAccount->setRequestedPresence(presence);

    QTRY_COMPARE(mAccount->status(), QString("away"));
    QCOMPARE(statusChangedSpy.count(), 1);

    // check that for a null account the status is null
    QVERIFY(mNullAccount->status().isNull());
}

void AccountEntryTest::testStatusMessage()
{
    QSignalSpy statusMessageChangedSpy(mAccount, SIGNAL(statusMessageChanged()));

    // check that the value is correct already
    QTRY_COMPARE(mAccount->statusMessage(), mTpAccount->connection()->selfContact()->presence().statusMessage());

    // and now set a new value
    QString statusMessage("I am online");
    Tp::Presence presence(Tp::ConnectionPresenceTypeAvailable, "available", statusMessage);
    mTpAccount->setRequestedPresence(presence);

    QTRY_COMPARE(mAccount->statusMessage(), statusMessage);
    QCOMPARE(statusMessageChangedSpy.count(), 1);

    // check that for a null account the displayName is null
    QVERIFY(mNullAccount->statusMessage().isNull());
}

void AccountEntryTest::testConnected()
{
    QSignalSpy connectedChangedSpy(mAccount, SIGNAL(connectedChanged()));

    // the mock account is enabled/connected by default, so make sure it is like that
    QVERIFY(mAccount->connected());

    // now set the account offline and see if the active flag changes correctly
    mMockController->setOnline(false);
    QTRY_VERIFY(!mAccount->connected());
    QCOMPARE(connectedChangedSpy.count(), 1);

    // now re-enable the account and check that the entry is updated
    connectedChangedSpy.clear();
    mMockController->setOnline(true);
    QTRY_VERIFY(mAccount->connected());
    QCOMPARE(connectedChangedSpy.count(), 1);

    // check that for a null account the displayName is null
    QVERIFY(!mNullAccount->connected());
}

void AccountEntryTest::testCompareIds_data()
{
    QTest::addColumn<QString>("first");
    QTest::addColumn<QString>("second");
    QTest::addColumn<bool>("expectedResult");

    QTest::newRow("identical values") << "1234567" << "1234567" << true;
    QTest::newRow("case difference") << "TestId" << "testid" << false;
    QTest::newRow("phone prefix") << "1234567" << "1231234567" << false;
}

void AccountEntryTest::testCompareIds()
{
    QFETCH(QString, first);
    QFETCH(QString, second);
    QFETCH(bool, expectedResult);

    QCOMPARE(mAccount->compareIds(first, second), expectedResult);
}

QTEST_MAIN(AccountEntryTest)
#include "AccountEntryTest.moc"
