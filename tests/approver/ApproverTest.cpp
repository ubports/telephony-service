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
    void testSnapDecisionTimeout();
    void testAcceptCall();
    void testCarKitOutgoingCall();

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

void ApproverTest::testSnapDecisionTimeout()
{
    QString callerId("12345");
    QVariantMap properties;
    properties["Caller"] = callerId;
    properties["State"] = "incoming";

    QDBusInterface notificationsMock("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QSignalSpy notificationSpy(&notificationsMock, SIGNAL(MockNotificationReceived(QString, uint, QString, QString, QString, QStringList, QVariantMap, int)));
    mMockController->placeCall(properties);
    TRY_COMPARE(notificationSpy.count(), 1);
    QVariantMap hints = notificationSpy.first()[6].toMap();
    QVERIFY(hints.contains("x-canonical-snap-decisions-timeout"));
    QCOMPARE(hints["x-canonical-snap-decisions-timeout"].toInt(), -1);
    mMockController->HangupCall(callerId);
}

void ApproverTest::testAcceptCall()
{
    QString callerId("7654321");

    QVariantMap properties;
    properties["Caller"] = callerId;
    properties["State"] = "incoming";

    QDBusInterface notificationsMock("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QSignalSpy notificationSpy(&notificationsMock, SIGNAL(MockNotificationReceived(QString, uint, QString, QString, QString, QStringList, QVariantMap, int)));
    QString objectPath = mMockController->placeCall(properties);
    TRY_COMPARE(notificationSpy.count(), 1);

    // at this point we are already sure the approver has the call, as the notification was placed
    QSignalSpy callStateSpy(mMockController, SIGNAL(CallStateChanged(QString,QString,QString)));
    ApproverController::instance()->acceptCall();
    TRY_COMPARE(callStateSpy.count(), 1);
    QCOMPARE(callStateSpy.first()[0].toString(), callerId);
    QCOMPARE(callStateSpy.first()[1].toString(), objectPath);
    QCOMPARE(callStateSpy.first()[2].toString(), QString("accepted"));
    mMockController->HangupCall(callerId);
}

void ApproverTest::testCarKitOutgoingCall()
{
    // make sure that an outgoing call placed outside of telepathy is handle correctly
    QString callerId("2345678");

    QVariantMap properties;
    properties["Caller"] = callerId;
    properties["State"] = "outgoing";

    QSignalSpy callStateSpy(mMockController, SIGNAL(CallStateChanged(QString,QString,QString)));

    QDBusInterface notificationsMock("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QSignalSpy notificationSpy(&notificationsMock, SIGNAL(MockNotificationReceived(QString, uint, QString, QString, QString, QStringList, QVariantMap, int)));
    QString objectPath = mMockController->placeCall(properties);

    // wait for a few seconds and check that no notification was displayed
    QTest::qWait(3000);
    QCOMPARE(notificationSpy.count(), 0);

    // check that the call state is not "accepted"
    TRY_VERIFY(callStateSpy.count() > 0);
    QCOMPARE(callStateSpy.last()[0].toString(), callerId);
    QCOMPARE(callStateSpy.last()[1].toString(), objectPath);
    QCOMPARE(callStateSpy.last()[2].toString(), QString("initialized"));
    mMockController->HangupCall(callerId);
}

QTEST_MAIN(ApproverTest)
#include "ApproverTest.moc"
