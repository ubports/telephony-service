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
#include "telepathyhelper.h"

// inheriting just to make the constructor public for testing
class TelepathyHelperWrapper : public TelepathyHelper
{
    Q_OBJECT
public:
    TelepathyHelperWrapper(QObject *parent = 0)
        : TelepathyHelper(parent) { }
};

class TelepathyHelperSetupTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanup();
    void testTelepathyHelperReadyNoAccounts();
    void testTelepathyHelperReady();

protected:
    Tp::AccountPtr addAccountAndWait(const QString &manager,
                                     const QString &protocol,
                                     const QString &displayName,
                                     const QVariantMap &parameters = QVariantMap());
private:
    Tp::AccountPtr mGenericTpAccount1;
    Tp::AccountPtr mGenericTpAccount2;
    Tp::AccountPtr mPhoneTpAccount1;
    Tp::AccountPtr mPhoneTpAccount2;
};

void TelepathyHelperSetupTest::initTestCase()
{
    initialize();
}

void TelepathyHelperSetupTest::testTelepathyHelperReadyNoAccounts()
{
    TelepathyHelperWrapper telepathyHelper;
    QSignalSpy setupReadySpy(&telepathyHelper, SIGNAL(setupReady()));
    TRY_COMPARE(setupReadySpy.count(), 1);

    QVERIFY(telepathyHelper.ready());

    QCOMPARE(telepathyHelper.accounts().size(), 0);
}

void TelepathyHelperSetupTest::testTelepathyHelperReady()
{
    // add four accounts
    mGenericTpAccount1 = addAccount("mock", "mock", "the generic account 1");
    QVERIFY(!mGenericTpAccount1.isNull());

    mGenericTpAccount2 = addAccount("mock", "mock", "the generic account 2");
    QVERIFY(!mGenericTpAccount2.isNull());

    mPhoneTpAccount1 = addAccount("mock", "ofono", "the phone account 1");
    QVERIFY(!mPhoneTpAccount1.isNull());

    mPhoneTpAccount2 = addAccount("mock", "ofono", "the phone account 2");
    QVERIFY(!mPhoneTpAccount2.isNull());

    TelepathyHelperWrapper telepathyHelper;
    QSignalSpy setupReadySpy(&telepathyHelper, SIGNAL(setupReady()));
    TRY_COMPARE(setupReadySpy.count(), 1);

    QVERIFY(telepathyHelper.ready());

    QCOMPARE(telepathyHelper.accounts().size(), 4);

    Q_FOREACH(const AccountEntry* account, telepathyHelper.accounts()) {
         QVERIFY(account->ready());
    }
}

void TelepathyHelperSetupTest::cleanup()
{
    // the accounts are removed in the parent class.
    doCleanup();
}

QTEST_MAIN(TelepathyHelperSetupTest)
#include "TelepathyHelperSetupTest.moc"
