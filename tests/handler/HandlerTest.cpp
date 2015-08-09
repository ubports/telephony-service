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
#include "handlercontroller.h"
#include "mockcontroller.h"
#include "approver.h"
#include "accountentry.h"
#include "accountentryfactory.h"
#include "telepathyhelper.h"

class HandlerTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testMakingCalls();
    void testHangUpCall();
    void testCallHold();
    void testCallProperties();
    void testConferenceCall();
    void testSendMessage();
    void testActiveCallIndicator();
    void testNotApprovedChannels();

private:
    void registerApprover();
    void unregisterApprover();
    void waitForCallActive(const QString &callerId);
    Approver *mApprover;
    MockController *mMockController;
    Tp::AccountPtr mTpAccount;
};

void HandlerTest::initTestCase()
{
    initialize();

    QSignalSpy setupReadySpy(TelepathyHelper::instance(), SIGNAL(setupReady()));
    TRY_COMPARE(setupReadySpy.count(), 1);

    registerApprover();
}

void HandlerTest::init()
{
    mTpAccount = addAccount("mock", "mock", "the account");

    // and create the mock controller
    mMockController = new MockController("mock", this);
}

void HandlerTest::cleanup()
{
    doCleanup();
    mMockController->deleteLater();
}

void HandlerTest::testMakingCalls()
{
    QString callerId("1234567");
    QSignalSpy callReceivedSpy(mMockController, SIGNAL(CallReceived(QString)));
    // FIXME: add support for multiple accounts
    HandlerController::instance()->startCall(callerId, mTpAccount->uniqueIdentifier());
    TRY_COMPARE(callReceivedSpy.count(), 1);
    QCOMPARE(callReceivedSpy.first().first().toString(), callerId);

    mMockController->HangupCall(callerId);
}

void HandlerTest::testHangUpCall()
{
    QString callerId("7654321");

    QVariantMap properties;
    properties["Caller"] = callerId;
    properties["State"] = "incoming";

    QSignalSpy approverCallSpy(mApprover, SIGNAL(newCall()));
    QString objectPath = mMockController->placeCall(properties);
    QVERIFY(!objectPath.isEmpty());

    // wait for the channel to hit the approver
    TRY_COMPARE(approverCallSpy.count(), 1);
    mApprover->acceptCall();

    waitForCallActive(callerId);

    // and finally request the hangup
    QSignalSpy callEndedSpy(mMockController, SIGNAL(CallEnded(QString)));
    HandlerController::instance()->hangUpCall(objectPath);
    TRY_COMPARE(callEndedSpy.count(), 1);
}

void HandlerTest::testCallHold()
{
    QString callerId("44444444");

    QVariantMap properties;
    properties["Caller"] = callerId;
    properties["State"] = "incoming";

    QSignalSpy approverCallSpy(mApprover, SIGNAL(newCall()));
    QString objectPath = mMockController->placeCall(properties);
    QVERIFY(!objectPath.isEmpty());

    // wait for the channel to hit the approver
    TRY_COMPARE(approverCallSpy.count(), 1);
    mApprover->acceptCall();

    waitForCallActive(callerId);

    QSignalSpy callStateSpy(mMockController, SIGNAL(CallStateChanged(QString,QString,QString)));

    // set the call on hold
    HandlerController::instance()->setHold(objectPath, true);
    TRY_COMPARE(callStateSpy.count(), 1);
    QCOMPARE(callStateSpy.first()[2].toString(), QString("held"));

    // and now set it as unheld again
    callStateSpy.clear();
    HandlerController::instance()->setHold(objectPath, false);
    TRY_COMPARE(callStateSpy.count(), 1);
    QCOMPARE(callStateSpy.first()[2].toString(), QString("active"));

    mMockController->HangupCall(callerId);
}

void HandlerTest::testCallProperties()
{
    QString callerId("7654321");

    QVariantMap properties;
    properties["Caller"] = callerId;
    properties["State"] = "incoming";

    QSignalSpy approverCallSpy(mApprover, SIGNAL(newCall()));
    QSignalSpy handlerCallPropertiesSpy(HandlerController::instance(), SIGNAL(callPropertiesChanged(QString,QVariantMap)));
    mMockController->placeCall(properties);

    // wait for the channel to hit the approver
    TRY_COMPARE(approverCallSpy.count(), 1);
    mApprover->acceptCall();

    waitForCallActive(callerId);

    // wait until the call properties are changed
    TRY_VERIFY(handlerCallPropertiesSpy.count() > 0);
    QString objectPath = handlerCallPropertiesSpy.last()[0].toString();
    QVariantMap propsFromSignal = handlerCallPropertiesSpy.last()[1].toMap();
    QVERIFY(!propsFromSignal.isEmpty());
    QDateTime activeTimestampFromSignal;
    QDateTime timestampFromSignal;
    propsFromSignal["activeTimestamp"].value<QDBusArgument>() >> activeTimestampFromSignal;
    propsFromSignal["timestamp"].value<QDBusArgument>() >> timestampFromSignal;
    QVERIFY(activeTimestampFromSignal.isValid());
    QVERIFY(timestampFromSignal.isValid());

    // and try to get the properties using the method
    QVariantMap propsFromMethod = HandlerController::instance()->getCallProperties(objectPath);
    QVERIFY(!propsFromMethod.isEmpty());
    QDateTime activeTimestampFromMethod;
    QDateTime timestampFromMethod;
    propsFromMethod["activeTimestamp"].value<QDBusArgument>() >> activeTimestampFromMethod;
    propsFromMethod["timestamp"].value<QDBusArgument>() >> timestampFromMethod;
    QCOMPARE(activeTimestampFromSignal, activeTimestampFromMethod);
    QCOMPARE(timestampFromSignal, timestampFromMethod);

    // now send some DTMF tones and check if the property is properly updated
    handlerCallPropertiesSpy.clear();
    QString dtmfString("1234*#");
    for (int i = 0; i < dtmfString.length(); ++i) {
        HandlerController::instance()->sendDTMF(objectPath, QString(dtmfString[i]));
    }
    TRY_COMPARE(handlerCallPropertiesSpy.count(), dtmfString.length());
    propsFromSignal = handlerCallPropertiesSpy.last()[1].toMap();
    propsFromMethod = HandlerController::instance()->getCallProperties(objectPath);
    QString dtmfStringFromSignal = propsFromSignal["dtmfString"].toString();
    QString dtmfStringFromMethod = propsFromMethod["dtmfString"].toString();
    QCOMPARE(dtmfStringFromSignal, dtmfString);
    QCOMPARE(dtmfStringFromMethod, dtmfString);

    HandlerController::instance()->hangUpCall(objectPath);
    QTest::qWait(500);
}

void HandlerTest::testConferenceCall()
{
    QString callerId1("55555555");
    QString callerId2("66666666");
    QString callerId3("77777777");

    QVariantMap properties;
    properties["Caller"] = callerId1;
    properties["State"] = "incoming";

    QSignalSpy approverCallSpy(mApprover, SIGNAL(newCall()));
    QString call1 = mMockController->placeCall(properties);

    // wait for the channel to hit the approver
    TRY_COMPARE(approverCallSpy.count(), 1);
    mApprover->acceptCall();
    approverCallSpy.clear();
    waitForCallActive(callerId1);

    // make a second call
    properties["Caller"] = callerId2;
    QString call2 = mMockController->placeCall(properties);
    // wait for the channel to hit the approver
    TRY_COMPARE(approverCallSpy.count(), 1);
    mApprover->acceptCall();
    approverCallSpy.clear();
    waitForCallActive(callerId2);

    // now create the conf call
    QSignalSpy conferenceCreatedSpy(mMockController, SIGNAL(ConferenceCreated(QString)));
    HandlerController::instance()->createConferenceCall(QStringList() << call1 << call2);
    TRY_COMPARE(conferenceCreatedSpy.count(), 1);
    QString conferenceObjectPath = conferenceCreatedSpy.first().first().toString();

    // now place a third call and try to merge it
    properties["Caller"] = callerId3;
    QString call3 = mMockController->placeCall(properties);
    TRY_COMPARE(approverCallSpy.count(), 1);
    mApprover->acceptCall();
    approverCallSpy.clear();
    waitForCallActive(callerId3);

    // merge that call on the conference
    QSignalSpy channelMergedSpy(mMockController, SIGNAL(ChannelMerged(QString)));
    HandlerController::instance()->mergeCall(conferenceObjectPath, call3);
    TRY_COMPARE(channelMergedSpy.count(), 1);
    QCOMPARE(channelMergedSpy.first().first().toString(), call3);

    // now try to split one of the channels
    QSignalSpy channelSplittedSpy(mMockController, SIGNAL(ChannelSplitted(QString)));
    HandlerController::instance()->splitCall(call2);
    TRY_COMPARE(channelSplittedSpy.count(), 1);
    QCOMPARE(channelSplittedSpy.first().first().toString(), call2);

    // now hangup the conference and the individual channels
    HandlerController::instance()->hangUpCall(conferenceObjectPath);
    HandlerController::instance()->hangUpCall(call1);
    HandlerController::instance()->hangUpCall(call2);
    HandlerController::instance()->hangUpCall(call3);

    // just to make sure it is all done before removing the accounts
    QTest::qWait(500);
}

void HandlerTest::testSendMessage()
{
    QString recipient("22222222");
    QString message("Hello, world!");
    QSignalSpy messageSentSpy(mMockController, SIGNAL(MessageSent(QString,QVariantMap)));
    // FIXME: add support for multiple accounts
    HandlerController::instance()->sendMessage(mTpAccount->uniqueIdentifier(), QStringList() << recipient, message);
    TRY_COMPARE(messageSentSpy.count(), 1);
    QString sentMessage = messageSentSpy.first().first().toString();
    QVariantMap messageProperties = messageSentSpy.first().last().value<QVariantMap>();
    QCOMPARE(sentMessage, message);
    QCOMPARE(messageProperties["Recipients"].value<QStringList>().count(), 1);
    QCOMPARE(messageProperties["Recipients"].value<QStringList>().first(), recipient);
}

void HandlerTest::testActiveCallIndicator()
{
    // start by making sure the property is false by default
    QVERIFY(!HandlerController::instance()->callIndicatorVisible());
    QSignalSpy spy(HandlerController::instance(), SIGNAL(callIndicatorVisibleChanged(bool)));

    // set the property to true
    HandlerController::instance()->setCallIndicatorVisible(true);
    TRY_COMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toBool());
    QVERIFY(HandlerController::instance()->callIndicatorVisible());

    // and back to false
    spy.clear();
    HandlerController::instance()->setCallIndicatorVisible(false);
    TRY_COMPARE(spy.count(), 1);
    QVERIFY(!spy.first().first().toBool());
    QVERIFY(!HandlerController::instance()->callIndicatorVisible());
}

void HandlerTest::testNotApprovedChannels()
{
    QVariantMap properties;
    QString callerId = "123456";
    properties["Caller"] = callerId;
    properties["State"] = "incoming";

    QSignalSpy approverCallSpy(mApprover, SIGNAL(newCall()));

    QSignalSpy callStateSpy(mMockController, SIGNAL(CallStateChanged(QString,QString,QString)));
    QString objectPath = mMockController->placeCall(properties);
    QVERIFY(!objectPath.isEmpty());

    // wait for the channel to hit the approver
    TRY_COMPARE(approverCallSpy.count(), 1);

    // accept the call but do not call callChannel->accept() on the channel
    mApprover->acceptCall(false);

    // wait for a few seconds
    QTest::qWait(3000);

    // the last state received should be initialised
    TRY_VERIFY(callStateSpy.count() > 0);
    QCOMPARE(callStateSpy.last()[0].toString(), callerId);
    QCOMPARE(callStateSpy.last()[1].toString(), objectPath);
    QCOMPARE(callStateSpy.last()[2].toString(), QString("initialised"));
}

void HandlerTest::registerApprover()
{
    // register the approver
    mApprover = new Approver();
    QVERIFY(TelepathyHelper::instance()->registerClient(mApprover, "TelephonyTestApprover"));
    // Tp-qt does not set registered status to approvers
    TRY_VERIFY(QDBusConnection::sessionBus().interface()->isServiceRegistered(TELEPHONY_SERVICE_APPROVER));
}

void HandlerTest::unregisterApprover()
{
    QVERIFY(TelepathyHelper::instance()->unregisterClient(mApprover));
    mApprover->deleteLater();
    mApprover = NULL;
    TRY_VERIFY(!QDBusConnection::sessionBus().interface()->isServiceRegistered(TELEPHONY_SERVICE_APPROVER));
}

void HandlerTest::waitForCallActive(const QString &callerId)
{
    QSignalSpy callStateSpy(mMockController, SIGNAL(CallStateChanged(QString,QString,QString)));
    QString state;
    QString objectPath;
    QString caller;
    int tries = 0;
    while ((state != "active" || caller != callerId) && tries < 5) {
        TRY_COMPARE(callStateSpy.count(), 1);
        caller = callStateSpy.first()[0].toString();
        objectPath = callStateSpy.first()[1].toString();
        state = callStateSpy.first()[2].toString();
        callStateSpy.clear();
        tries++;
        qDebug() << "Waiting for call active, try " << tries << " failed.";
    }

    QCOMPARE(caller, callerId);
    QCOMPARE(state, QString("active"));
    QVERIFY(!objectPath.isEmpty());
}

QTEST_MAIN(HandlerTest)
#include "HandlerTest.moc"
