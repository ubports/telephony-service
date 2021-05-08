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
#include "telepathytest.h"
#include "accountentry.h"
#include "chatmanager.h"
#include "telepathyhelper.h"
#include "mockcontroller.h"

Q_DECLARE_METATYPE(Tp::TextChannelPtr)

class ChatManagerTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testSendMessage_data();
    void testSendMessage();
    void testSendMessageWithAttachments();
    void testSendMessageWithAttachmentsSplitted();
    void testAcknowledgeMessages();
    void testRedownloadMessage();

private:
    Tp::AccountPtr mGenericTpAccount;
    Tp::AccountPtr mPhoneTpAccount;
    Tp::AccountPtr mNuntiumTpAccount;
    MockController *mGenericMockController;
    MockController *mPhoneMockController;
    MockController *mNuntiumMockController;
};

void ChatManagerTest::initTestCase()
{
    initialize();
    TelepathyHelper::instance()->registerChannelObserver();

    qRegisterMetaType<Tp::TextChannelPtr>();

    // just give telepathy some time to register the observer
    QTest::qWait(1000);
}

void ChatManagerTest::init()
{
    // add two accounts
    mGenericTpAccount = addAccount("mock", "mock", "the generic account");
    QVERIFY(!mGenericTpAccount.isNull());
    mPhoneTpAccount = addAccount("mock", "ofono", "the phone account");
    QVERIFY(!mPhoneTpAccount.isNull());
    mNuntiumTpAccount = addAccount("mock", "nuntium", "the nuntium acccount");
    QVERIFY(!mNuntiumTpAccount.isNull());

    // and create the mock controller
    mGenericMockController = new MockController("mock", this);
    mPhoneMockController = new MockController("ofono", this);
    mNuntiumMockController = new MockController("nuntium", this);
}

void ChatManagerTest::cleanup()
{
    // the accounts are removed in the parent class.
    doCleanup();

    mGenericMockController->deleteLater();
    mPhoneMockController->deleteLater();
    mNuntiumMockController->deleteLater();
}

void ChatManagerTest::testSendMessage_data()
{
    QTest::addColumn<QStringList>("recipients");
    QTest::addColumn<QString>("message");
    QTest::addColumn<QString>("accountId");

    QTest::newRow("message via the generic account") << (QStringList() << "recipient1") << QString("Hello world") << QString("mock/mock/account0");
    QTest::newRow("message via the phone account") << (QStringList() << "recipient2") << QString("Hello Phone World") << QString("mock/ofono/account0");
    QTest::newRow("multiple recipients") << (QStringList() << "123" << "456" << "789") << QString("Hi friends!") << QString("mock/mock/account0");
}

void ChatManagerTest::testSendMessage()
{
    QFETCH(QStringList, recipients);
    QFETCH(QString, message);
    QFETCH(QString, accountId);

    // just to make it easier, sort the recipients
    qSort(recipients);

    MockController *controller = accountId.startsWith("mock/mock") ? mGenericMockController : mPhoneMockController;
    QSignalSpy controllerMessageSentSpy(controller, SIGNAL(MessageSent(QString,QVariantList,QVariantMap)));

    QVariantMap properties;
    properties["participantIds"] = recipients;
    QString jobObjectPath = ChatManager::instance()->sendMessage(accountId, message, QVariantMap(), properties);

    QDBusInterface iface(TelepathyHelper::instance()->handlerInterface()->service(),
                         jobObjectPath);

    QSignalSpy finishedSpy(&iface, SIGNAL(finished()));

    TRY_COMPARE(controllerMessageSentSpy.count(), 1);
    QString messageText = controllerMessageSentSpy.first()[0].toString();
    QVariantMap messageProperties = controllerMessageSentSpy.first()[2].toMap();
    QStringList messageRecipients = messageProperties["Recipients"].toStringList();
    qSort(messageRecipients);
    QCOMPARE(messageText, message);
    QCOMPARE(messageRecipients, recipients);

    // the rest of the properties are tested in the MessageSendingJob tests.
    TRY_COMPARE(finishedSpy.count(), 1);
}

void ChatManagerTest::testAcknowledgeMessages()
{
    QSignalSpy textChannelAvailableSpy(ChatManager::instance(), SIGNAL(textChannelAvailable(Tp::TextChannelPtr)));

    QVariantMap properties;
    properties["Sender"] = "12345";
    properties["Recipients"] = (QStringList() << "12345");
    QStringList messages;
    messages << "Hi there" << "How are you" << "Always look on the bright side of life";
    Q_FOREACH(const QString &message, messages) {
        mGenericMockController->PlaceIncomingMessage(message, properties);
        // the wait shouldn't be needed, but just in case
        QTest::qWait(50);
    }
    TRY_COMPARE(textChannelAvailableSpy.count(), 1);
    Tp::TextChannelPtr channel = textChannelAvailableSpy.first().first().value<Tp::TextChannelPtr>();
    QVERIFY(!channel.isNull());

    TRY_COMPARE(channel->messageQueue().count(), messages.count());
    QStringList messageIds;
    for (int i = 0; i < messages.count(); ++i) {
        messageIds << channel->messageQueue()[i].messageToken();
    }

    QSignalSpy messageReadSpy(mGenericMockController, SIGNAL(MessageRead(QString)));
    QVariantMap ackProperties;
    ackProperties["accountId"] = "mock/mock/account0";
    ackProperties["participantIds"] = properties["Recipients"].toStringList();
    Q_FOREACH(const QString &messageId, messageIds) {
        ackProperties["messageId"] = messageId;
        ChatManager::instance()->acknowledgeMessage(ackProperties);
    }

    TRY_COMPARE(messageReadSpy.count(), messageIds.count());
    QStringList receivedIds;
    for (int i = 0; i < messageReadSpy.count(); ++i) {
        receivedIds << messageReadSpy[i][0].toString();
    }

    qSort(receivedIds);
    qSort(messageIds);
    QCOMPARE(receivedIds, messageIds);
}

void ChatManagerTest::testRedownloadMessage()
{
    TRY_COMPARE(1, 2);
    //QSignalSpy textChannelAvailableSpy(ChatManager::instance(), SIGNAL(textChannelAvailable(Tp::TextChannelPtr)));

    //QVariantMap properties;
    //properties["Sender"] = "12345";
    //properties["Recipients"] = (QStringList() << "12345");
    //QStringList messages;
    //messages << "Hi there" << "How are you" << "Always look on the bright side of life";
    //TODO:jezek - Place this incomming message as temporaily failed.
    //Q_FOREACH(const QString &message, messages) {
    //    mGenericMockController->PlaceIncomingMessage(message, properties);
    //    // the wait shouldn't be needed, but just in case
    //    QTest::qWait(50);
    //}
    //TRY_COMPARE(textChannelAvailableSpy.count(), 1);
    //Tp::TextChannelPtr channel = textChannelAvailableSpy.first().first().value<Tp::TextChannelPtr>();
    //QVERIFY(!channel.isNull());

    //TRY_COMPARE(channel->messageQueue().count(), messages.count());
    //QStringList messageIds;
    //for (int i = 0; i < messages.count(); ++i) {
    //    messageIds << channel->messageQueue()[i].messageToken();
    //}

    ////TODO:jezek - Check for dbus signal "Redownload".
    //QSignalSpy messageReadSpy(mGenericMockController, SIGNAL(MessageRead(QString)));
    //Q_FOREACH(const QString &messageId, messageIds) {
    //    //TODO:jezek 12345 is not right, use threadId.
    //    ChatManager::instance()->redownloadMessage("mock/mock/account0", "12345", messageId );
    //}

    //TRY_COMPARE(messageReadSpy.count(), messageIds.count());
    //QStringList receivedIds;
    //for (int i = 0; i < messageReadSpy.count(); ++i) {
    //    receivedIds << messageReadSpy[i][0].toString();
    //}

    //qSort(receivedIds);
    //qSort(messageIds);
    //QCOMPARE(receivedIds, messageIds);
    ////TODO:jezek - Check if message has pending state.
}

void ChatManagerTest::testSendMessageWithAttachments()
{
    QStringList recipients = (QStringList() << "1234567");
    QString message("Hello Phone Attachments World");
    QString accountId("mock/ofono/account0");

    MockController *controller = mPhoneMockController;

    QSignalSpy controllerMessageSentSpy(controller, SIGNAL(MessageSent(QString,QVariantList,QVariantMap)));

    QVariantList attachmentList;
    QVariantList attachment;
    attachment << "id" << "content/type" << QString("%1/%2").arg(QString(qgetenv("TEST_DATA_DIR"))).arg("dialer-app.png");
    attachmentList << QVariant::fromValue(attachment);
    QVariant attachments = QVariant::fromValue(attachmentList);

    QVariantMap properties;
    properties["participantIds"] = recipients;
    ChatManager::instance()->sendMessage(accountId, message, attachments, properties);

    TRY_COMPARE(controllerMessageSentSpy.count(), 1);

    QString messageText = controllerMessageSentSpy.first()[0].toString();
    QVariantList messageAttachments = controllerMessageSentSpy.first()[1].toList();
    QVariantMap messageProperties = controllerMessageSentSpy.first()[2].toMap();
    QStringList messageRecipients = messageProperties["Recipients"].toStringList();
    qSort(messageRecipients);
    QCOMPARE(messageText, message);
    QCOMPARE(messageRecipients, recipients);
    QCOMPARE(messageAttachments.count(), attachmentList.count());
}

void ChatManagerTest::testSendMessageWithAttachmentsSplitted()
{
    // messages sent on accounts other than phone are splitted, so make sure that happens
    QStringList recipients = (QStringList() << "theattachmentrecipient");
    QString message("Hello Attachments World");
    QString accountId("mock/mock/account0");

    MockController *controller = mGenericMockController;

    QSignalSpy controllerMessageSentSpy(controller, SIGNAL(MessageSent(QString,QVariantList,QVariantMap)));

    QVariantList attachmentList;
    QVariantList attachment;
    attachment << "id" << "content/type" << QString("%1/%2").arg(QString(qgetenv("TEST_DATA_DIR"))).arg("dialer-app.png");
    attachmentList << QVariant::fromValue(attachment);
    QVariant attachments = QVariant::fromValue(attachmentList);

    QVariantMap properties;
    properties["participantIds"] = recipients;
    ChatManager::instance()->sendMessage(accountId, message, attachments, properties);

    TRY_COMPARE(controllerMessageSentSpy.count(), attachmentList.count() + 1);

    QString messageText;
    int attachmentCount = 0;
    // as the message is splitted, we have to go through all the received messages to find the
    // text and attachments
    for (int i = 0; i < controllerMessageSentSpy.count(); ++i) {
        QList<QVariant> args = controllerMessageSentSpy[i];
        // validate the recipients on all messages
        QVariantMap messageProperties = args[2].toMap();
        QStringList messageRecipients = messageProperties["Recipients"].toStringList();
        qSort(messageRecipients);
        QCOMPARE(messageRecipients, recipients);
        QString text = args[0].toString();
        if (!text.isEmpty()) {
            messageText = text;
        } else {
            QVariantList attachments = args[1].toList();
            // each message should contain no more than one attachment
            QCOMPARE(attachments.count(), 1);
            attachmentCount += 1;
        }
    }
    QCOMPARE(messageText, message);
    QCOMPARE(attachmentCount, attachmentList.count());

}

QTEST_MAIN(ChatManagerTest)
#include "ChatManagerTest.moc"
