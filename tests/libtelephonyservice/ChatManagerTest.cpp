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
#include "accountentry.h"
#include "chatmanager.h"
#include "telepathyhelper.h"
#include "mockcontroller.h"

class ChatManagerTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testSendMessage_data();
    void testSendMessage();
    void testSendMessageWithAttachments_data();
    void testSendMessageWithAttachments();
    void testMessageReceived();
    void testAcknowledgeMessages();
    void testChatEntry();

private:
    Tp::AccountPtr mGenericTpAccount;
    Tp::AccountPtr mPhoneTpAccount;
    MockController *mGenericMockController;
    MockController *mPhoneMockController;
};

void ChatManagerTest::initTestCase()
{
    initialize();
    // set to false so group chat messages are not sent as MMS
    mPhoneSettings.set("mmsGroupChatEnabled", false);
    TelepathyHelper::instance()->registerChannelObserver();

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

    // and create the mock controller
    mGenericMockController = new MockController("mock", this);
    mPhoneMockController = new MockController("ofono", this);
}

void ChatManagerTest::cleanup()
{
    // the accounts are removed in the parent class.
    doCleanup();

    mGenericMockController->deleteLater();
    mPhoneMockController->deleteLater();
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
    QSignalSpy messageSentSpy(ChatManager::instance(), SIGNAL(messageSent(QStringList,QString)));

    ChatManager::instance()->sendMessage(accountId, recipients, message);

    TRY_COMPARE(controllerMessageSentSpy.count(), 1);
    QString messageText = controllerMessageSentSpy.first()[0].toString();
    QVariantMap messageProperties = controllerMessageSentSpy.first()[2].toMap();
    QStringList messageRecipients = messageProperties["Recipients"].toStringList();
    qSort(messageRecipients);
    QCOMPARE(messageText, message);
    QCOMPARE(messageRecipients, recipients);

    TRY_COMPARE(messageSentSpy.count(), 1);
    messageRecipients = messageSentSpy.first()[0].toStringList();
    qSort(messageRecipients);
    messageText = messageSentSpy.first()[1].toString();
    QCOMPARE(messageText, message);
    QCOMPARE(messageRecipients, recipients);
}

void ChatManagerTest::testMessageReceived()
{
    QSignalSpy messageReceivedSpy(ChatManager::instance(), SIGNAL(messageReceived(QString,QString,QDateTime,QString,bool)));

    QVariantMap properties;
    properties["Sender"] = "12345";
    properties["Recipients"] = (QStringList() << "12345");
    QString message("Hi there");
    mGenericMockController->PlaceIncomingMessage(message, properties);

    TRY_COMPARE(messageReceivedSpy.count(), 1);
    QString sender = messageReceivedSpy.first()[0].toString();
    QString receivedMessage = messageReceivedSpy.first()[1].toString();
    QCOMPARE(sender, properties["Sender"].toString());
    QCOMPARE(receivedMessage, message);
}

void ChatManagerTest::testAcknowledgeMessages()
{
    QSignalSpy messageReceivedSpy(ChatManager::instance(), SIGNAL(messageReceived(QString,QString,QDateTime,QString,bool)));

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
    TRY_COMPARE(messageReceivedSpy.count(), messages.count());

    QStringList messageIds;
    for (int i = 0; i < messages.count(); ++i) {
        QString messageId = messageReceivedSpy[i][3].toString();
        messageIds << messageId;
    }

    QSignalSpy messageReadSpy(mGenericMockController, SIGNAL(MessageRead(QString)));
    Q_FOREACH(const QString &messageId, messageIds) {
        ChatManager::instance()->acknowledgeMessage(properties["Recipients"].toStringList(), messageId, "mock/mock/account0");
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

void ChatManagerTest::testChatEntry()
{
    QStringList recipients;
    recipients << "user@domain.com" << "user2@domain.com";
    QSignalSpy chatEntryCreatedSpy(ChatManager::instance(), SIGNAL(chatEntryCreated(QString, QStringList,ChatEntry *)));
    ChatEntry *entry = ChatManager::instance()->chatEntryForParticipants("mock/mock/account0", recipients, true);
    QVERIFY(entry == NULL);
    QTRY_COMPARE(chatEntryCreatedSpy.count(), 1);

    entry = ChatManager::instance()->chatEntryForParticipants("mock/mock/account0", recipients, false);
    QVERIFY(entry != NULL);
    QList<QVariant> arguments = chatEntryCreatedSpy.takeFirst();
    QCOMPARE(QString("mock/mock/account0"), arguments.at(0).toString());
    QCOMPARE(recipients.toSet(), arguments.at(1).toStringList().toSet());
    QCOMPARE(entry, arguments.at(2).value<ChatEntry*>());
}

void ChatManagerTest::testSendMessageWithAttachments_data()
{
    QTest::addColumn<QStringList>("recipients");
    QTest::addColumn<QString>("message");
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<bool>("mmsFlag");

    QTest::newRow("message via the generic account") << (QStringList() << "recipient1") << QString("Hello world") << QString("mock/mock/account0") << false;
    QTest::newRow("message via the phone account") << (QStringList() << "1234567") << QString("Hello Phone World") << QString("mock/ofono/account0") << true;

}

void ChatManagerTest::testSendMessageWithAttachments()
{
    QFETCH(QStringList, recipients);
    QFETCH(QString, message);
    QFETCH(QString, accountId);
    QFETCH(bool, mmsFlag);

    // just to make it easier, sort the recipients
    qSort(recipients);

    MockController *controller = accountId.startsWith("mock/mock") ? mGenericMockController : mPhoneMockController;

    QSignalSpy controllerMessageSentSpy(controller, SIGNAL(MessageSent(QString,QVariantList,QVariantMap)));

    QVariantList attachmentList;
    QVariantList attachment;
    attachment << "id" << "content/type" << "filepath";
    attachmentList << QVariant::fromValue(attachment);
    QVariant attachments = QVariant::fromValue(attachmentList);

    ChatManager::instance()->sendMessage(accountId, recipients, message, attachments);

    TRY_COMPARE(controllerMessageSentSpy.count(), 1);
    QString messageText = controllerMessageSentSpy.first()[0].toString();
    QVariantMap messageProperties = controllerMessageSentSpy.first()[2].toMap();
    QStringList messageRecipients = messageProperties["Recipients"].toStringList();
    qSort(messageRecipients);
    QCOMPARE(messageText, message);
    QCOMPARE(messageRecipients, recipients);

    QCOMPARE(messageProperties.contains("x-canonical-mms"), mmsFlag);
    QCOMPARE(messageProperties["x-canonical-mms"].toBool(), mmsFlag);
}

QTEST_MAIN(ChatManagerTest)
#include "ChatManagerTest.moc"
