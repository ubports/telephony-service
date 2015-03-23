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
#include "ofonoaccountentry.h"
#include "telepathyhelper.h"

class AccountEntryFactoryTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCorrectInstancesCreated();
    void testNullAccount();
};

void AccountEntryFactoryTest::initTestCase()
{
    initialize();
}

void AccountEntryFactoryTest::testCorrectInstancesCreated()
{
    Tp::AccountPtr genericTpAccount = addAccount("mock", "mock", "generic");
    QVERIFY(!genericTpAccount.isNull());
    AccountEntry *genericAccount = AccountEntryFactory::createEntry(genericTpAccount, this);
    QVERIFY(genericAccount);
    QCOMPARE(genericAccount->type(), AccountEntry::GenericAccount);
    QVERIFY(!qobject_cast<OfonoAccountEntry*>(genericAccount));

    Tp::AccountPtr ofonoTpAccount = addAccount("mock", "ofono", "phone account");
    AccountEntry *ofonoAccount = AccountEntryFactory::createEntry(ofonoTpAccount, this);
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
