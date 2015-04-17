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
#include "ofonoaccountentry.h"
#include "accountentryfactory.h"
#include "telepathyhelper.h"
#include "mockcontroller.h"

class USSDManagerTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

private:
    OfonoAccountEntry *mAccount;
    Tp::AccountPtr mTpAccount;
    MockController *mMockController;
};

void USSDManagerTest::initTestCase()
{
    initialize();
}

void USSDManagerTest::init()
{
    mTpAccount = addAccount("mock", "ofono", "the account");
    QVERIFY(!mTpAccount.isNull());
    QTRY_VERIFY(mTpAccount->isReady(Tp::Account::FeatureCore));

    mAccount = qobject_cast<OfonoAccountEntry*>(AccountEntryFactory::createEntry(mTpAccount, this));
    QVERIFY(mAccount);

    // make sure the connection is available
    QTRY_VERIFY(!mTpAccount->connection().isNull());
    QTRY_COMPARE(mTpAccount->connection()->selfContact()->presence().type(), Tp::ConnectionPresenceTypeAvailable);
    QTRY_VERIFY(mAccount->connected());

    // and create the mock controller
    mMockController = new MockController("ofono", this);

    // just in case, wait some time
    QTest::qWait(500);
}

void USSDManagerTest::cleanup()
{
    doCleanup();

    mAccount->deleteLater();
    mMockController->deleteLater();
}

QTEST_MAIN(USSDManagerTest)
#include "USSDManagerTest.moc"
