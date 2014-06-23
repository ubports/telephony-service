/*
 * Copyright (C) 2013 Canonical, Ltd.
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
#include "handlercontroller.h"
#include "mockcontroller.h"
#include "approver.h"
#include "telepathyhelper.h"

class HandlerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testMakingCalls();
    void testHangUpCall();
    void testCallHold();
    void testCallProperties();
    void testConferenceCall();
    void testSendMessage();
    void testActiveCallIndicator();

private:
    void waitForCallActive(const QString &callerId);
    Approver *mApprover;
};

void HandlerTest::initTestCase()
{
    QSignalSpy spy(TelepathyHelper::instance(), SIGNAL(accountReady()));
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, 10000);
    QTRY_VERIFY_WITH_TIMEOUT(TelepathyHelper::instance()->connected(), 10000);

    // register the approver
    mApprover = new Approver(this);
    TelepathyHelper::instance()->registerClient(mApprover, "TelephonyTestApprover");
    // Tp-qt does not set registered status to approvers
    QTRY_VERIFY(QDBusConnection::sessionBus().interface()->isServiceRegistered(TELEPHONY_SERVICE_APPROVER));

    // we need to wait in order to give telepathy time to notify about the approver
    QTest::qWait(3000);
}

void HandlerTest::testMakingCalls()
{
    QString callerId("1234567");
    QSignalSpy callReceivedSpy(MockController::instance(), SIGNAL(callReceived(QString)));
    // FIXME: add support for multiple accounts
    HandlerController::instance()->startCall(callerId, TelepathyHelper::instance()->accountId());
    QTRY_COMPARE(callReceivedSpy.count(), 1);
    QCOMPARE(callReceivedSpy.first().first().toString(), callerId);

    MockController::instance()->hangupCall(callerId);
}

void HandlerTest::testHangUpCall()
{
    QString callerId("7654321");

    QVariantMap properties;
    properties["Caller"] = callerId;
    properties["State"] = "incoming";

    QSignalSpy approverCallSpy(mApprover, SIGNAL(newCall()));
    QString objectPath = MockController::instance()->placeCall(properties);
    QVERIFY(!objectPath.isEmpty());

    // wait for the channel to hit the approver
    QTRY_COMPARE(approverCallSpy.count(), 1);
    mApprover->acceptCall();

    waitForCallActive(callerId);

    // and finally request the hangup
    QSignalSpy callEndedSpy(MockController::instance(), SIGNAL(callEnded(QString)));
    HandlerController::instance()->hangUpCall(objectPath);
    QTRY_COMPARE(callEndedSpy.count(), 1);
}

void HandlerTest::testCallHold()
{
    QString callerId("44444444");

    QVariantMap properties;
    properties["Caller"] = callerId;
    properties["State"] = "incoming";

    QSignalSpy approverCallSpy(mApprover, SIGNAL(newCall()));
    QString objectPath = MockController::instance()->placeCall(properties);
    QVERIFY(!objectPath.isEmpty());

    // wait for the channel to hit the approver
    QTRY_COMPARE(approverCallSpy.count(), 1);
    mApprover->acceptCall();

    waitForCallActive(callerId);

    QSignalSpy callStateSpy(MockController::instance(), SIGNAL(callStateChanged(QString,QString,QString)));

    // set the call on hold
    HandlerController::instance()->setHold(objectPath, true);
    QTRY_COMPARE(callStateSpy.count(), 1);
    QCOMPARE(callStateSpy.first()[2].toString(), QString("held"));

    // and now set it as unheld again
    callStateSpy.clear();
    HandlerController::instance()->setHold(objectPath, false);
    QTRY_COMPARE(callStateSpy.count(), 1);
    QCOMPARE(callStateSpy.first()[2].toString(), QString("active"));

    MockController::instance()->hangupCall(callerId);
}

void HandlerTest::testCallProperties()
{
    QString callerId("7654321");

    QVariantMap properties;
    properties["Caller"] = callerId;
    properties["State"] = "incoming";

    QSignalSpy approverCallSpy(mApprover, SIGNAL(newCall()));
    QSignalSpy handlerCallPropertiesSpy(HandlerController::instance(), SIGNAL(callPropertiesChanged(QString,QVariantMap)));
    MockController::instance()->placeCall(properties);

    // wait for the channel to hit the approver
    QTRY_COMPARE(approverCallSpy.count(), 1);
    mApprover->acceptCall();

    // wait until the call properties are changed
    QTRY_VERIFY(handlerCallPropertiesSpy.count() > 0);
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
    QTRY_COMPARE(handlerCallPropertiesSpy.count(), dtmfString.length());
    propsFromSignal = handlerCallPropertiesSpy.last()[1].toMap();
    propsFromMethod = HandlerController::instance()->getCallProperties(objectPath);
    QString dtmfStringFromSignal = propsFromSignal["dtmfString"].toString();
    QString dtmfStringFromMethod = propsFromMethod["dtmfString"].toString();
    QCOMPARE(dtmfStringFromSignal, dtmfString);
    QCOMPARE(dtmfStringFromMethod, dtmfString);

    HandlerController::instance()->hangUpCall(objectPath);
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
    QString call1 = MockController::instance()->placeCall(properties);

    // wait for the channel to hit the approver
    QTRY_COMPARE(approverCallSpy.count(), 1);
    mApprover->acceptCall();
    approverCallSpy.clear();
    waitForCallActive(callerId1);

    // make a second call
    properties["Caller"] = callerId2;
    QString call2 = MockController::instance()->placeCall(properties);
    // wait for the channel to hit the approver
    QTRY_COMPARE(approverCallSpy.count(), 1);
    mApprover->acceptCall();
    approverCallSpy.clear();
    waitForCallActive(callerId2);

    // now create the conf call
    QSignalSpy conferenceCreatedSpy(MockController::instance(), SIGNAL(conferenceCreated(QString)));
    HandlerController::instance()->createConferenceCall(QStringList() << call1 << call2);
    QTRY_COMPARE(conferenceCreatedSpy.count(), 1);
    QString conferenceObjectPath = conferenceCreatedSpy.first().first().toString();

    // now place a third call and try to merge it
    properties["Caller"] = callerId3;
    QString call3 = MockController::instance()->placeCall(properties);
    QTRY_COMPARE(approverCallSpy.count(), 1);
    mApprover->acceptCall();
    approverCallSpy.clear();
    waitForCallActive(callerId3);

    // merge that call on the conference
    QSignalSpy channelMergedSpy(MockController::instance(), SIGNAL(channelMerged(QString)));
    HandlerController::instance()->mergeCall(conferenceObjectPath, call3);
    QTRY_COMPARE(channelMergedSpy.count(), 1);
    QCOMPARE(channelMergedSpy.first().first().toString(), call3);

    // now try to split one of the channels
    QSignalSpy channelSplittedSpy(MockController::instance(), SIGNAL(channelSplitted(QString)));
    HandlerController::instance()->splitCall(call2);
    QTRY_COMPARE(channelSplittedSpy.count(), 1);
    QCOMPARE(channelSplittedSpy.first().first().toString(), call2);

    // now hangup the conference and the individual channels
    HandlerController::instance()->hangUpCall(conferenceObjectPath);
    HandlerController::instance()->hangUpCall(call1);
    HandlerController::instance()->hangUpCall(call2);
    HandlerController::instance()->hangUpCall(call3);
}

void HandlerTest::testSendMessage()
{
    QString recipient("22222222");
    QString message("Hello, world!");
    QSignalSpy messageSentSpy(MockController::instance(), SIGNAL(messageSent(QString,QVariantMap)));
    // FIXME: add support for multiple accounts
    HandlerController::instance()->sendMessage(recipient, message, TelepathyHelper::instance()->accountId());
    QTRY_COMPARE(messageSentSpy.count(), 1);
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
    QTRY_COMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toBool());
    QVERIFY(HandlerController::instance()->callIndicatorVisible());

    // and back to false
    spy.clear();
    HandlerController::instance()->setCallIndicatorVisible(false);
    QTRY_COMPARE(spy.count(), 1);
    QVERIFY(!spy.first().first().toBool());
    QVERIFY(!HandlerController::instance()->callIndicatorVisible());
}

void HandlerTest::waitForCallActive(const QString &callerId)
{
    // wait until the call state is "accepted"
    QSignalSpy callStateSpy(MockController::instance(), SIGNAL(callStateChanged(QString,QString,QString)));
    QString state;
    QString objectPath;
    QString caller;
    int tries = 0;
    while (state != "active" && caller != callerId && tries < 5) {
        QTRY_COMPARE(callStateSpy.count(), 1);
        caller = callStateSpy.first()[0].toString();
        objectPath = callStateSpy.first()[1].toString();
        state = callStateSpy.first()[2].toString();
        callStateSpy.clear();
        tries++;
    }

    QCOMPARE(caller, callerId);
    QCOMPARE(state, QString("active"));
    QVERIFY(!objectPath.isEmpty());
}

QTEST_MAIN(HandlerTest)
#include "HandlerTest.moc"
