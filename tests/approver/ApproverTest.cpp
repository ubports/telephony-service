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
#include "telepathytest.h"
#include "approvercontroller.h"
#include "mockcontroller.h"
#include "accountentry.h"
#include "accountentryfactory.h"
#include "telepathyhelper.h"

class ApproverTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testAcceptCall();

private:
    void waitForCallActive(const QString &callerId);
    MockController *mMockController;
    Tp::AccountPtr mTpAccount;
};

void ApproverTest::initTestCase()
{
    initialize();

    QSignalSpy setupReadySpy(TelepathyHelper::instance(), SIGNAL(setupReady()));
    TRY_COMPARE(setupReadySpy.count(), 1);

    TRY_VERIFY(QDBusConnection::sessionBus().interface()->isServiceRegistered("com.canonical.Approver"));
}

void ApproverTest::init()
{
    mTpAccount = addAccount("mock", "mock", "the account");

    // and create the mock controller
    mMockController = new MockController("mock", this);
}

void ApproverTest::cleanup()
{
    doCleanup();
    mMockController->deleteLater();
}

void ApproverTest::testAcceptCall()
{
    QString callerId("7654321");

    QVariantMap properties;
    properties["Caller"] = callerId;
    properties["State"] = "incoming";
    mMockController->placeCall(properties);

    // we don't have a reliable way to check if the call hit the approver yet
    QTest::qWait(5000);

    qDebug() << "Accepting the call";
    QSignalSpy callStateSpy(mMockController, SIGNAL(CallStateChanged(QString,QString,QString)));
    ApproverController::instance()->acceptCall();
    TRY_COMPARE(callStateSpy.count(), 1);
    QCOMPARE(callStateSpy.first()[0].toString(), callerId);
    QCOMPARE(callStateSpy.first()[1].toString(), QString("accepted"));
}

QTEST_MAIN(ApproverTest)
#include "ApproverTest.moc"
