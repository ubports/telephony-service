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
#include "telepathytest.h"
#include "accountentry.h"
#include "accountentryfactory.h"
#include "mockcontroller.h"

class AccountEntryTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testAccountId();
    void testActive();
    void testDisplayName();
    void testStatus();
    void testStatusMessage();
    void testConnected();
    void testCompareIds_data();
    void testCompareIds();
    void testAddressableVCardFields();

private:
    AccountEntry *mAccount;
    Tp::AccountPtr mTpAccount;
    AccountEntry *mNullAccount;
    MockController *mMockController;
};

void AccountEntryTest::initTestCase()
{
    initialize();

    // create a null account
    mNullAccount = AccountEntryFactory::createEntry(Tp::AccountPtr(), this);
}

void AccountEntryTest::init()
{
    mTpAccount = addAccount("mock", "mock", "the account");
    QVERIFY(!mTpAccount.isNull());

    mAccount = AccountEntryFactory::createEntry(mTpAccount, this);
    QVERIFY(mAccount);
    TRY_VERIFY(mAccount->ready());

    // make sure the connection is available
    TRY_VERIFY(mAccount->connected());

    // and make sure the status and status message are the ones we expect
    TRY_COMPARE(mAccount->status(), QString("available"));
    TRY_COMPARE(mAccount->statusMessage(), QString("online"));

    // and create the mock controller
    mMockController = new MockController("mock", this);
}

void AccountEntryTest::cleanup()
{
    doCleanup();

    mAccount->deleteLater();
    mMockController->deleteLater();
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

    // now set the account away and see if the active flag changes correctly
    mMockController->SetOnline(false);
    TRY_VERIFY(!mAccount->active());
    TRY_VERIFY(activeChangedSpy.count() > 0);


    // now re-enable the account and check that the entry is updated
    activeChangedSpy.clear();
    mMockController->SetOnline(true);
    TRY_VERIFY(activeChangedSpy.count() > 0);
    QVERIFY(mAccount->active());

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
    TRY_COMPARE(mAccount->displayName(), newDisplayName);
    QCOMPARE(displayNameChangedSpy.count(), 1);

    // and try setting the display name in the entry itself
    displayNameChangedSpy.clear();
    newDisplayName = "changing again";
    mAccount->setDisplayName(newDisplayName);
    TRY_COMPARE(mAccount->displayName(), newDisplayName);
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
    mMockController->SetPresence("away", "away");

    TRY_COMPARE(statusChangedSpy.count(), 1);
    QCOMPARE(mAccount->status(), QString("away"));

    // check that for a null account the status is null
    QVERIFY(mNullAccount->status().isNull());
}

void AccountEntryTest::testStatusMessage()
{
    QSignalSpy statusMessageChangedSpy(mAccount, SIGNAL(statusMessageChanged()));

    // check that the value is correct already
    TRY_COMPARE(mAccount->statusMessage(), mTpAccount->connection()->selfContact()->presence().statusMessage());

    // and now set a new value
    QString statusMessage("I am online");
    mMockController->SetPresence("available", statusMessage);

    TRY_COMPARE(statusMessageChangedSpy.count(), 1);
    QCOMPARE(mAccount->statusMessage(), statusMessage);

    // check that for a null account the displayName is null
    QVERIFY(mNullAccount->statusMessage().isNull());
}

void AccountEntryTest::testConnected()
{
    QSignalSpy connectedChangedSpy(mAccount, SIGNAL(connectedChanged()));

    // the mock account is enabled/connected by default, so make sure it is like that
    QVERIFY(mAccount->connected());

    // now set the account offline and see if the active flag changes correctly
    mMockController->SetPresence("away", "away");
    TRY_VERIFY(connectedChangedSpy.count() > 0);
    TRY_VERIFY(!mAccount->connected());

    // now re-enable the account and check that the entry is updated
    connectedChangedSpy.clear();
    mMockController->SetPresence("available", "online");
    // because of the way the mock was implemented, sometimes this can return two connectedChanged() signals.
    TRY_VERIFY(connectedChangedSpy.count() > 0);
    TRY_VERIFY(mAccount->connected());

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

void AccountEntryTest::testAddressableVCardFields()
{
    QVERIFY(!mAccount->addressableVCardFields().isEmpty());
    QCOMPARE(mAccount->addressableVCardFields(), mTpAccount->protocolInfo().addressableVCardFields());
}

QTEST_MAIN(AccountEntryTest)
#include "AccountEntryTest.moc"
