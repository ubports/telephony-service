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
#include "ussdmanager.h"

class USSDManagerTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testInitiate();
    void testRespond();
    void testCancel();
    void testActive_data();
    void testActive();
    void testActiveChanged();
    void testState();
    void testNotificationReceived();
    void testRequestReceived();
    void testInitiateUSSDComplete();
    void testRespondComplete_data();
    void testRespondComplete();
    void testBarringComplete();
    void testForwardingComplete();
    void testWaitingComplete();
    void testCallingLinePresentationComplete();
    void testConnectedLinePresentationComplete();
    void testCallingLineRestrictionComplete();
    void testConnectedLineRestrictionComplete();
    void testInitiateFailed();
    void testAccountReconnect();

private:
    OfonoAccountEntry *mAccount;
    Tp::AccountPtr mTpAccount;
    MockController *mMockController;
    USSDManager *mManager;
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
    mManager = new USSDManager(mAccount, mAccount);
}

void USSDManagerTest::cleanup()
{
    doCleanup();

    mAccount->deleteLater();
    mMockController->deleteLater();
    mManager->deleteLater();
}

void USSDManagerTest::testInitiate()
{
    QString command("initiatecommand");
    QSignalSpy initiateSpy(mMockController, SIGNAL(USSDInitiateCalled(QString)));
    mManager->initiate(command);
    QTRY_COMPARE(initiateSpy.count(), 1);
    QCOMPARE(initiateSpy.first()[0].toString(), command);
}

void USSDManagerTest::testRespond()
{
    QString reply("somereply");
    QSignalSpy respondSpy(mMockController, SIGNAL(USSDRespondCalled(QString)));
    mManager->respond(reply);
    QTRY_COMPARE(respondSpy.count(), 1);
    QCOMPARE(respondSpy.first()[0].toString(), reply);
}

void USSDManagerTest::testCancel()
{
    QSignalSpy cancelSpy(mMockController, SIGNAL(USSDCancelCalled()));
    mManager->cancel();
    QTRY_COMPARE(cancelSpy.count(), 1);
}

void USSDManagerTest::testActive_data()
{
    QTest::addColumn<QString>("state");
    QTest::addColumn<bool>("active");

    QTest::newRow("idle is inactive") << "idle" << false;
    QTest::newRow("active is active") << "active" << true;
    QTest::newRow("anything else is active") << "blabla" << true;
}

void USSDManagerTest::testActive()
{
    QFETCH(QString, state);
    QFETCH(bool, active);
    mMockController->TriggerUSSDStateChanged(state);
    QTRY_COMPARE(mManager->active(), active);
}

void USSDManagerTest::testActiveChanged()
{
    QSignalSpy activeSpy(mManager, SIGNAL(activeChanged()));
    mMockController->TriggerUSSDStateChanged("active");
    QTRY_COMPARE(activeSpy.count(), 1);
}

void USSDManagerTest::testState()
{
    QString state = "foobar";
    QSignalSpy stateSpy(mManager, SIGNAL(stateChanged(QString)));
    mMockController->TriggerUSSDStateChanged(state);
    QTRY_COMPARE(stateSpy.count(), 1);
    QCOMPARE(stateSpy.first()[0].toString(), state);
    QCOMPARE(mManager->state(), state);
}

void USSDManagerTest::testNotificationReceived()
{
    QString notification("the notification");
    QSignalSpy notificationReceivedSpy(mManager, SIGNAL(notificationReceived(QString)));
    mMockController->TriggerUSSDNotificationReceived(notification);
    QTRY_COMPARE(notificationReceivedSpy.count(), 1);
    QCOMPARE(notificationReceivedSpy.first()[0].toString(), notification);
}

void USSDManagerTest::testRequestReceived()
{
    QString request("the notification");
    QSignalSpy requestReceivedSpy(mManager, SIGNAL(requestReceived(QString)));
    mMockController->TriggerUSSDRequestReceived(request);
    QTRY_COMPARE(requestReceivedSpy.count(), 1);
    QCOMPARE(requestReceivedSpy.first()[0].toString(), request);
}

void USSDManagerTest::testInitiateUSSDComplete()
{
    QString response("some response");
    QSignalSpy initiateUSSDCompleteSpy(mManager, SIGNAL(initiateUSSDComplete(QString)));
    mMockController->TriggerUSSDInitiateUSSDComplete(response);
    QTRY_COMPARE(initiateUSSDCompleteSpy.count(), 1);
    QCOMPARE(initiateUSSDCompleteSpy.first()[0].toString(), response);
}

void USSDManagerTest::testRespondComplete_data()
{
    QTest::addColumn<bool>("success");
    QTest::addColumn<QString>("response");

    QTest::newRow("success") << true << "somesuccessvalue";
    QTest::newRow("failure") << false << "somefailurevalue";
}

void USSDManagerTest::testRespondComplete()
{
    QFETCH(bool, success);
    QFETCH(QString, response);
    QSignalSpy respondCompleteSpy(mManager, SIGNAL(respondComplete(bool,QString)));
    mMockController->TriggerUSSDRespondComplete(success, response);
    QTRY_COMPARE(respondCompleteSpy.count(), 1);
    QCOMPARE(respondCompleteSpy.first()[0].toBool(), success);
    QCOMPARE(respondCompleteSpy.first()[1].toString(), response);
}

void USSDManagerTest::testBarringComplete()
{
    QString op("theOp");
    QString service("theService");
    QVariantMap map;
    map["op"] = op;
    map["service"] = service;

    QSignalSpy barringCompleteSpy(mManager, SIGNAL(barringComplete(QString,QString,QVariantMap)));
    mMockController->TriggerUSSDBarringComplete(op, service, map);
    QTRY_COMPARE(barringCompleteSpy.count(), 1);
    QCOMPARE(barringCompleteSpy.first()[0].toString(), op);
    QCOMPARE(barringCompleteSpy.first()[1].toString(), service);
    QCOMPARE(barringCompleteSpy.first()[2].toMap(), map);
}

void USSDManagerTest::testForwardingComplete()
{
    QString op("theOtherOp");
    QString service("theOtherService");
    QVariantMap map;
    map["op"] = op;
    map["service"] = service;
    map["count"] = 1;

    QSignalSpy forwardingCompleteSpy(mManager, SIGNAL(forwardingComplete(QString,QString,QVariantMap)));
    mMockController->TriggerUSSDForwardingComplete(op, service, map);
    QTRY_COMPARE(forwardingCompleteSpy.count(), 1);
    QCOMPARE(forwardingCompleteSpy.first()[0].toString(), op);
    QCOMPARE(forwardingCompleteSpy.first()[1].toString(), service);
    QCOMPARE(forwardingCompleteSpy.first()[2].toMap(), map);
}

void USSDManagerTest::testWaitingComplete()
{
    QString op("anotherOp");
    QVariantMap map;
    map["op"] = op;
    map["one"] = 1;
    map["two"] = 2;

    QSignalSpy waitingCompleteSpy(mManager, SIGNAL(waitingComplete(QString,QVariantMap)));
    mMockController->TriggerUSSDWaitingComplete(op, map);
    QTRY_COMPARE(waitingCompleteSpy.count(), 1);
    QCOMPARE(waitingCompleteSpy.first()[0].toString(), op);
    QCOMPARE(waitingCompleteSpy.first()[1].toMap(), map);
}

void USSDManagerTest::testCallingLinePresentationComplete()
{
    QString op("clpOp");
    QString status("clpStatus");

    QSignalSpy clpCompleteSpy(mManager, SIGNAL(callingLinePresentationComplete(QString,QString)));
    mMockController->TriggerUSSDCallingLinePresentationComplete(op, status);
    QTRY_COMPARE(clpCompleteSpy.count(), 1);
    QCOMPARE(clpCompleteSpy.first()[0].toString(), op);
    QCOMPARE(clpCompleteSpy.first()[1].toString(), status);
}

void USSDManagerTest::testConnectedLinePresentationComplete()
{
    QString op("clp2Op");
    QString status("clp2Status");

    QSignalSpy clpCompleteSpy(mManager, SIGNAL(connectedLinePresentationComplete(QString,QString)));
    mMockController->TriggerUSSDConnectedLinePresentationComplete(op, status);
    QTRY_COMPARE(clpCompleteSpy.count(), 1);
    QCOMPARE(clpCompleteSpy.first()[0].toString(), op);
    QCOMPARE(clpCompleteSpy.first()[1].toString(), status);
}

void USSDManagerTest::testCallingLineRestrictionComplete()
{
    QString op("clrOp");
    QString status("clrStatus");

    QSignalSpy clrCompleteSpy(mManager, SIGNAL(callingLineRestrictionComplete(QString,QString)));
    mMockController->TriggerUSSDCallingLineRestrictionComplete(op, status);
    QTRY_COMPARE(clrCompleteSpy.count(), 1);
    QCOMPARE(clrCompleteSpy.first()[0].toString(), op);
    QCOMPARE(clrCompleteSpy.first()[1].toString(), status);
}

void USSDManagerTest::testConnectedLineRestrictionComplete()
{
    QString op("clr2Op");
    QString status("clr2Status");

    QSignalSpy clrCompleteSpy(mManager, SIGNAL(connectedLineRestrictionComplete(QString,QString)));
    mMockController->TriggerUSSDConnectedLineRestrictionComplete(op, status);
    QTRY_COMPARE(clrCompleteSpy.count(), 1);
    QCOMPARE(clrCompleteSpy.first()[0].toString(), op);
    QCOMPARE(clrCompleteSpy.first()[1].toString(), status);
}

void USSDManagerTest::testInitiateFailed()
{
    QSignalSpy initiateFailedSpy(mManager, SIGNAL(initiateFailed()));
    mMockController->TriggerUSSDInitiateFailed();
    QTRY_COMPARE(initiateFailedSpy.count(), 1);
}

void USSDManagerTest::testAccountReconnect()
{
    // make sure that methods and signals are working after an account reconnects
    Q_EMIT mAccount->connectedChanged();

    // and now try one method call
    QString command("reinitiatecommand");
    QSignalSpy initiateSpy(mMockController, SIGNAL(USSDInitiateCalled(QString)));
    mManager->initiate(command);
    QTRY_COMPARE(initiateSpy.count(), 1);
    QCOMPARE(initiateSpy.first()[0].toString(), command);

    // and check one of the signals
    QString state("someCorrectState");
    QSignalSpy stateChangedSpy(mManager, SIGNAL(stateChanged(QString)));
    mMockController->TriggerUSSDStateChanged(state);
    QTRY_COMPARE(stateChangedSpy.count(), 1);
    QCOMPARE(stateChangedSpy.first()[0].toString(), state);
    QCOMPARE(mManager->state(), state);

}

QTEST_MAIN(USSDManagerTest)
#include "USSDManagerTest.moc"
