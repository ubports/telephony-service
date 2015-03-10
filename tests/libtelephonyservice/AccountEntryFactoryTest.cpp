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
#include "ofonoaccountentry.h"
#include "telepathyhelper.h"

#define DEFAULT_TIMEOUT 15000

class AccountEntryFactoryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCorrectInstancesCreated();
    void testNullAccount();
};

void AccountEntryFactoryTest::initTestCase()
{
    Tp::registerTypes();

    QSignalSpy spy(TelepathyHelper::instance(), SIGNAL(setupReady()));
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, DEFAULT_TIMEOUT);
    QTRY_VERIFY_WITH_TIMEOUT(TelepathyHelper::instance()->connected(), DEFAULT_TIMEOUT);

    // we need to wait in order to give telepathy time to notify about the approver
    QTest::qWait(3000);
}

void AccountEntryFactoryTest::testCorrectInstancesCreated()
{
    AccountEntry *genericAccount = TelepathyHelper::instance()->accountForId("mock/mock/account0");
    QVERIFY(genericAccount);
    QCOMPARE(genericAccount->type(), AccountEntry::GenericAccount);
    QVERIFY(!qobject_cast<OfonoAccountEntry*>(genericAccount));

    AccountEntry *ofonoAccount = TelepathyHelper::instance()->accountForId("mock/ofono/account0");
    QVERIFY(ofonoAccount);
    QCOMPARE(ofonoAccount->type(), AccountEntry::PhoneAccount);
    QVERIFY(qobject_cast<OfonoAccountEntry*>(ofonoAccount));
}

void AccountEntryFactoryTest::testNullAccount()
{
    AccountEntry *nullAccount = AccountEntryFactory::createEntry(Tp::AccountPtr(), this);
    QVERIFY(nullAccount);
    QVERIFY(nullAccount->accountId().isNull());
}


QTEST_MAIN(AccountEntryFactoryTest)
#include "AccountEntryFactoryTest.moc"
