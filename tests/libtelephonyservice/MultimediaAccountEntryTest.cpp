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
#include "multimediaaccountentry.h"
#include "accountentryfactory.h"
#include "mockcontroller.h"

class MultimediaAccountEntryTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testAccountType();

private:
    MultimediaAccountEntry *mAccount;
    Tp::AccountPtr mMultimediaAccount;
    MockController *mMockController;
};

void MultimediaAccountEntryTest::initTestCase()
{
    initialize();
}

void MultimediaAccountEntryTest::init()
{
    mMultimediaAccount = addAccount("mock", "multimedia", "multimedia account");
    QVERIFY(!mMultimediaAccount.isNull());
    mAccount = qobject_cast<MultimediaAccountEntry*>(AccountEntryFactory::createEntry(mMultimediaAccount, this));
    QVERIFY(mAccount);
    TRY_VERIFY(mAccount->ready());

    // make sure the connection is available
    TRY_VERIFY(mAccount->connected());

    // and make sure the status and status message are the ones we expect
    TRY_COMPARE(mAccount->status(), QString("available"));

    // create the mock controller
    mMockController = new MockController("multimedia", this);
}

void MultimediaAccountEntryTest::cleanup()
{
    doCleanup();
    mAccount->deleteLater();
    mMockController->deleteLater();
}

void MultimediaAccountEntryTest::testAccountType()
{
    QCOMPARE(mAccount->type(), AccountEntry::MultimediaAccount);
}

QTEST_MAIN(MultimediaAccountEntryTest)
#include "MultimediaAccountEntryTest.moc"
