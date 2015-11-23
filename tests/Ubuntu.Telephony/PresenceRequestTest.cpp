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
#include <TelepathyQt/Presence>
#include "telepathytest.h"
#include "presencerequest.h"
#include "telepathyhelper.h"
#include "mockcontroller.h"
#include "accountentryfactory.h"

Q_DECLARE_METATYPE(AccountEntry*)

class PresenceRequestTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testContactPresenceChange_data();
    void testContactPresenceChange();

private:
    Tp::AccountPtr mTpAccount;
    AccountEntry *mAccount;
    MockController *mMockController;
};

void PresenceRequestTest::initTestCase()
{
    qRegisterMetaType<AccountEntry*>();
    initialize();
    TelepathyHelper::instance()->registerChannelObserver();
}

void PresenceRequestTest::init()
{
    QSignalSpy accountSpy(TelepathyHelper::instance(), SIGNAL(accountAdded(AccountEntry*)));
    mTpAccount = addAccount("mock", "mock", "the account");
    QVERIFY(!mTpAccount.isNull());
    TRY_COMPARE(accountSpy.count(), 1);
    mAccount = qobject_cast<AccountEntry*>(accountSpy.first().first().value<AccountEntry*>());
    QVERIFY(mAccount);
    TRY_VERIFY(mAccount->ready());
    TRY_COMPARE(mAccount->status(), QString("available"));

    // and create the mock controller
    mMockController = new MockController("mock", this);
}

void PresenceRequestTest::cleanup()
{
    doCleanup();

    mMockController->deleteLater();
    mAccount->deleteLater();
}

void PresenceRequestTest::testContactPresenceChange_data()
{
    QTest::addColumn<QString>("id");
    QTest::addColumn<int>("presenceType");
    QTest::addColumn<QString>("status");
    QTest::addColumn<QString>("statusMessage");

    QTest::newRow("contact 1 online") << "contact1@test.com" << (int)Tp::ConnectionPresenceTypeAvailable << "online" << "message 1";
    QTest::newRow("contact 1 offline") << "contact1@test.com" << (int)Tp::ConnectionPresenceTypeOffline << "offline" << "";
    QTest::newRow("contact 2 online") << "contact2@test.com" << (int)Tp::ConnectionPresenceTypeAvailable << "online" << "message 2";
    QTest::newRow("contact 2 offline") << "contact2@test.com" << (int)Tp::ConnectionPresenceTypeOffline << "offline" << "";
}

void PresenceRequestTest::testContactPresenceChange()
{
    QFETCH(QString, id);
    QFETCH(int, presenceType);
    QFETCH(QString, status);
    QFETCH(QString, statusMessage);

    PresenceRequest presenceRequest;
    QSignalSpy presenceRequestSpy(&presenceRequest, SIGNAL(typeChanged()));
    presenceRequest.setAccountId(mAccount->accountId());
    presenceRequest.setIdentifier(id);
    presenceRequest.componentComplete();

    mMockController->SetContactPresence(id, presenceType, status, statusMessage);
    TRY_COMPARE(presenceRequestSpy.count(), 1);

    TRY_COMPARE(presenceRequest.identifier(), id);
    TRY_COMPARE(presenceRequest.type(), (uint)presenceType);
    TRY_COMPARE(presenceRequest.status(), status);
    TRY_COMPARE(presenceRequest.statusMessage(), statusMessage);
}

QTEST_MAIN(PresenceRequestTest)
#include "PresenceRequestTest.moc"
