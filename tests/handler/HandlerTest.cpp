/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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
#include <QDBusInterface>
#include "telepathytest.h"
#include "chatmanager.h"
#include "handlercontroller.h"
#include "mockcontroller.h"
#include "approver.h"
#include "accountentry.h"
#include "accountentryfactory.h"
#include "telepathyhelper.h"
#include "protocolmanager.h"
#include <config.h>

Q_DECLARE_METATYPE(Tp::TextChannelPtr)

class HandlerTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testGetProtocols();
    void testGetProtocolsChangesThroughDBus();
    void testMakingCalls();
    void testHangUpCall();
    void testCallHold();
    void testCallProperties();
    void testConferenceCall();
    void testSendMessage();
    void testSendMessageWithAttachments();
    void testSendMessageOwnNumber();
    void testAcknowledgeMessage();
    void testAcknowledgeAllMessages();
    void testActiveCallIndicator();
    void testNotApprovedChannels();
    void testMultimediaFallback();

private:
    void registerApprover();
    void unregisterApprover();
    void waitForCallActive(const QString &callerId);
    Approver *mApprover;
    MockController *mMockController;
    Tp::AccountPtr mTpAccount;
    MockController *mMultimediaMockController;
    Tp::AccountPtr mMultimediaTpAccount;
    MockController *mOfonoMockController;
    Tp::AccountPtr mOfonoTpAccount;

};

void HandlerTest::initTestCase()
{
    qDBusRegisterMetaType<ProtocolList>();
    qDBusRegisterMetaType<ProtocolStruct>();

    initialize();

    QSignalSpy setupReadySpy(TelepathyHelper::instance(), SIGNAL(setupReady()));
    TRY_COMPARE(setupReadySpy.count(), 1);

    qRegisterMetaType<Tp::TextChannelPtr>();

    registerApprover();
}

void HandlerTest::init()
{
    mTpAccount = addAccount("mock", "mock", "the account");

    // and create the mock controller
    mMockController = new MockController("mock", this);

    mOfonoTpAccount = addAccount("mock", "ofono", "the account");
    mOfonoMockController = new MockController("ofono", this);

    mMultimediaTpAccount = addAccount("mock", "multimedia", "the account");
    mMultimediaMockController = new MockController("multimedia", this);
}

void HandlerTest::cleanup()
{
    doCleanup();
    mMockController->deleteLater();
    mMultimediaMockController->deleteLater();
    mOfonoMockController->deleteLater();
}

void HandlerTest::testGetProtocols()
{
    Protocols protocols = ProtocolManager::instance()->protocols();
    ProtocolList protocolList = HandlerController::instance()->getProtocols();
    for (int i = 0; i < protocols.count(); ++i) {
        QCOMPARE(protocols[i]->name(), protocolList.at(i).name);
    }
}

void HandlerTest::testGetProtocolsChangesThroughDBus()
{
    QSignalSpy protocolsChangedSpy(HandlerController::instance(), SIGNAL(protocolsChanged(ProtocolList)));

    QTemporaryFile f;
    f.setFileTemplate(protocolsDir() + "/");
    if (f.open()) {
        f.close();
    }

    QVERIFY(!(f.fileName().isEmpty()));
    QTRY_COMPARE(protocolsChangedSpy.count(), 1);
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
    TRY_VERIFY(HandlerController::instance()->hasCalls());
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

    TRY_VERIFY(HandlerController::instance()->hasCalls());
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
    
    TRY_VERIFY(HandlerController::instance()->hasCalls());

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
    QSignalSpy messageSentSpy(mMockController, SIGNAL(MessageSent(QString,QVariantList,QVariantMap)));
    // FIXME: add support for multiple accounts
    HandlerController::instance()->sendMessage(mTpAccount->uniqueIdentifier(), QStringList() << recipient, message);
    TRY_COMPARE(messageSentSpy.count(), 1);
    QString sentMessage = messageSentSpy.first().first().toString();
    QVariantMap messageProperties = messageSentSpy.first().last().value<QVariantMap>();
    QCOMPARE(sentMessage, message);
    QCOMPARE(messageProperties["Recipients"].value<QStringList>().count(), 1);
    QCOMPARE(messageProperties["Recipients"].value<QStringList>().first(), recipient);
}

void HandlerTest::testSendMessageWithAttachments()
{
    // just to avoid the account fallback, remove the multimedia account
    QVERIFY(removeAccount(mMultimediaTpAccount));

    QString recipient("22222222");
    QString message("Hello, world!");
    QSignalSpy messageSentSpy(mOfonoMockController, SIGNAL(MessageSent(QString,QVariantList,QVariantMap)));

    QTemporaryFile outputFile("audioXXXXXX.ogg");
    outputFile.open();
    AttachmentStruct attachment{"id", "audio/ogg", outputFile.fileName()};
    HandlerController::instance()->sendMessage(mOfonoTpAccount->uniqueIdentifier(), QStringList() << recipient, message, AttachmentList() << attachment);
    TRY_COMPARE(messageSentSpy.count(), 1);
    outputFile.close();

    QString sentMessage = messageSentSpy.first()[0].toString();
    QVariantMap messageProperties = messageSentSpy.first()[2].value<QVariantMap>();
    QCOMPARE(sentMessage, message);
    QCOMPARE(messageProperties["Recipients"].value<QStringList>().count(), 1);
    QCOMPARE(messageProperties["Recipients"].value<QStringList>().first(), recipient);

    QVariantList messageAttachments = qdbus_cast<QVariantList>(messageSentSpy.first()[1]);
    QVariantMap firstAttachment = qdbus_cast<QVariantMap>(messageAttachments.first());
    QCOMPARE(firstAttachment["content-type"].toString(), QString("audio/ogg"));
    QCOMPARE(firstAttachment["identifier"].toString(), QString("id"));
}

void HandlerTest::testSendMessageOwnNumber()
{
    QString recipient("84376666");
    QString message("Hello, world!");
    QSignalSpy messageSentSpy(mMockController, SIGNAL(MessageSent(QString,QVariantList,QVariantMap)));

    // first send a message to our own number
    HandlerController::instance()->sendMessage(mTpAccount->uniqueIdentifier(), QStringList() << mTpAccount->connection()->selfContact()->id(), message);
    TRY_COMPARE(messageSentSpy.count(), 1);
    QVariantMap messageProperties = messageSentSpy.first()[2].value<QVariantMap>();
    QCOMPARE(messageProperties["Recipients"].toStringList().first(), mTpAccount->connection()->selfContact()->id());

    messageSentSpy.clear();

    // then send to another number and check if old channels are not reused
    HandlerController::instance()->sendMessage(mTpAccount->uniqueIdentifier(), QStringList() << recipient, message);
    TRY_COMPARE(messageSentSpy.count(), 1);
    messageProperties = messageSentSpy.first()[2].value<QVariantMap>();
    QCOMPARE(messageProperties["Recipients"].toStringList().first(), recipient);
}

void HandlerTest::testAcknowledgeMessage()
{
    QString recipient("84376666");
    QString recipient2("+554184376666");
    QString message("Hello, world!");

    QSignalSpy messageSentSpy(mMockController, SIGNAL(MessageSent(QString,QVariantList,QVariantMap)));
    // first send a message to a certain number so the handler request one channel
    QString objectPath = HandlerController::instance()->sendMessage(mTpAccount->uniqueIdentifier(), QStringList() << recipient, message);
    QDBusInterface *sendingJob = new QDBusInterface(TelepathyHelper::instance()->handlerInterface()->service(), objectPath,
                                                    "com.canonical.TelephonyServiceHandler.MessageSendingJob");
    QSignalSpy handlerHasChannel(sendingJob, SIGNAL(finished()));

    TRY_COMPARE(messageSentSpy.count(), 1);
    TRY_COMPARE(handlerHasChannel.count(), 1);

    // if we register the observer before this test, other tests fail
    TelepathyHelper::instance()->registerChannelObserver("TelephonyServiceTests");
    TRY_VERIFY(QDBusConnection::sessionBus().interface()->isServiceRegistered(TP_QT_IFACE_CLIENT + ".TelephonyServiceTests"));
    QSignalSpy textChannelAvailableSpy(ChatManager::instance(), SIGNAL(textChannelAvailable(Tp::TextChannelPtr)));

    // now receive a message from a very similar number so CM creates another
    // channel and the handler needs to deal with both
    QVariantMap properties;
    properties["Sender"] = recipient2;
    properties["Recipients"] = (QStringList() << recipient2);
    mMockController->PlaceIncomingMessage(message, properties);

    TRY_COMPARE(textChannelAvailableSpy.count(), 2);
    Tp::TextChannelPtr channel = textChannelAvailableSpy[1].first().value<Tp::TextChannelPtr>();
    QVERIFY(!channel.isNull());
    TRY_COMPARE(channel->messageQueue().count(), 1);
    QString receivedMessageId = channel->messageQueue().first().messageToken();

    // then acknowledge the message that arrived in the second channel and make sure handler
    // does the right thing
    QSignalSpy messageReadSpy(mMockController, SIGNAL(MessageRead(QString)));
    QTest::qWait(1000);
    QVariantMap ackProperties;
    ackProperties["accountId"] = mTpAccount->uniqueIdentifier();
    ackProperties["participantIds"] = properties["Recipients"].toStringList();
    ackProperties["messageId"] = receivedMessageId;
    ChatManager::instance()->acknowledgeMessage(ackProperties);

    TRY_COMPARE(messageReadSpy.count(), 1);
    QCOMPARE(messageReadSpy.first()[0].toString(), receivedMessageId);
}

void HandlerTest::testAcknowledgeAllMessages()
{
    // FIXME: we assume the observer is already registered from the test above
    QString recipient("98437666");
    QString recipient2("+554198437666");
    QString message("Hello, world! %1");
    int messageCount = 10;
    QSignalSpy messageSentSpy(mMockController, SIGNAL(MessageSent(QString,QVariantList,QVariantMap)));

    // first send a message to a certain number so the handler request one channel
    HandlerController::instance()->sendMessage(mTpAccount->uniqueIdentifier(), QStringList() << recipient, message);
    TRY_COMPARE(messageSentSpy.count(), 1);

    QSignalSpy textChannelAvailableSpy(ChatManager::instance(), SIGNAL(textChannelAvailable(Tp::TextChannelPtr)));

    // now receive some messages from a very similar number so CM creates another
    // channel and the handler needs to deal with both
    QVariantMap properties;
    properties["Sender"] = recipient2;
    properties["Recipients"] = (QStringList() << recipient2);
    for (int i = 0; i < messageCount; ++i) {
        mMockController->PlaceIncomingMessage(message.arg(QString::number(i)), properties);
    }

    TRY_COMPARE(textChannelAvailableSpy.count(), 1);
    Tp::TextChannelPtr channel = textChannelAvailableSpy.first().first().value<Tp::TextChannelPtr>();
    QVERIFY(!channel.isNull());
    TRY_COMPARE(channel->messageQueue().count(), messageCount);
    QString receivedMessageId = channel->messageQueue().first().messageToken();

    // then acknowledge the messages that arrived in the second channel and make sure handler
    // does the right thing
    QTest::qWait(1000);
    QSignalSpy messageReadSpy(mMockController, SIGNAL(MessageRead(QString)));

    QVariantMap ackProperties;
    ackProperties["accountId"] = mTpAccount->uniqueIdentifier();
    ackProperties["participantIds"] = properties["Recipients"].toStringList();

    ChatManager::instance()->acknowledgeAllMessages(ackProperties);

    TRY_COMPARE(messageReadSpy.count(), messageCount);
}

//TODO:jezek - testRedownloadMessage, documentation, changelogs, etc...

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

void HandlerTest::testMultimediaFallback()
{
    QString recipient("22222222");
    QString message("Hello, world!");
    mMultimediaMockController->SetContactPresence(recipient, Tp::ConnectionPresenceTypeAvailable, "available", "");
    // We have to make sure the handler already has the new state
    QTest::qWait(1000);

    QSignalSpy messageSentOfonoSpy(mOfonoMockController, SIGNAL(MessageSent(QString,QVariantList,QVariantMap)));
    QSignalSpy messageSentMultimediaSpy(mMultimediaMockController, SIGNAL(MessageSent(QString,QVariantList,QVariantMap)));

    QString jobObjectPath = HandlerController::instance()->sendMessage(mOfonoTpAccount->uniqueIdentifier(), QStringList() << recipient, message);

    QDBusInterface jobInterface(TelepathyHelper::instance()->handlerInterface()->service(),
                               jobObjectPath);
    QSignalSpy finishedSpy(&jobInterface, SIGNAL(finished()));
    TRY_COMPARE(finishedSpy.count(), 1);
    QCOMPARE(jobInterface.property("accountId").toString(), mMultimediaTpAccount->uniqueIdentifier());
    TRY_COMPARE(messageSentMultimediaSpy.count(), 1);
    QCOMPARE(messageSentOfonoSpy.count(), 0);
    QString sentMessage = messageSentMultimediaSpy.first().first().toString();
    QVariantMap messageProperties = messageSentMultimediaSpy.first().last().value<QVariantMap>();
    QCOMPARE(sentMessage, message);
    QCOMPARE(messageProperties["Recipients"].value<QStringList>().count(), 1);
    QCOMPARE(messageProperties["Recipients"].value<QStringList>().first(), recipient);

    messageSentMultimediaSpy.clear();
    messageSentOfonoSpy.clear();
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
