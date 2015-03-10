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
#include "accountentry.h"
#include "chatmanager.h"
#include "telepathyhelper.h"
#include "mockcontroller.h"

#define DEFAULT_TIMEOUT 15000

class ChatManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testSendMessage_data();
    void testSendMessage();
    void testMessageReceived();
    void testAcknowledgeMessages();

private:
    MockController *mGenericMockController;
    MockController *mPhoneMockController;
};

void ChatManagerTest::initTestCase()
{
    Tp::registerTypes();

    QSignalSpy spy(TelepathyHelper::instance(), SIGNAL(setupReady()));
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, DEFAULT_TIMEOUT);
    QTRY_VERIFY_WITH_TIMEOUT(TelepathyHelper::instance()->connected(), DEFAULT_TIMEOUT);

    TelepathyHelper::instance()->registerChannelObserver();

    // just give telepathy some time to register the observer
    QTest::qWait(1000);

    // and create the mock controller
    mGenericMockController = new MockController("mock", this);
    mPhoneMockController = new MockController("ofono", this);
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

    MockController *controller = (accountId == "mock/mock/account0") ? mGenericMockController : mPhoneMockController;
    QSignalSpy controllerMessageSentSpy(controller, SIGNAL(messageSent(QString,QVariantMap)));
    QSignalSpy messageSentSpy(ChatManager::instance(), SIGNAL(messageSent(QStringList,QString)));

    ChatManager::instance()->sendMessage(recipients, message, accountId);

    QTRY_COMPARE(controllerMessageSentSpy.count(), 1);
    QString messageText = controllerMessageSentSpy.first()[0].toString();
    QVariantMap messageProperties = controllerMessageSentSpy.first()[1].toMap();
    QStringList messageRecipients = messageProperties["Recipients"].toStringList();
    qSort(messageRecipients);
    QCOMPARE(messageText, message);
    QCOMPARE(messageRecipients, recipients);

    QTRY_COMPARE(messageSentSpy.count(), 1);
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
    mGenericMockController->placeIncomingMessage(message, properties);

    QTRY_COMPARE(messageReceivedSpy.count(), 1);
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
        mGenericMockController->placeIncomingMessage(message, properties);
        QTest::qWait(100);
    }
    QTRY_COMPARE(messageReceivedSpy.count(), messages.count());

    QStringList messageIds;
    for (int i = 0; i < messages.count(); ++i) {
        QString messageId = messageReceivedSpy[i][3].toString();
        messageIds << messageId;
    }

    QSignalSpy messageReadSpy(mGenericMockController, SIGNAL(messageRead(QString)));
    Q_FOREACH(const QString &messageId, messageIds) {
        ChatManager::instance()->acknowledgeMessage(properties["Recipients"].toStringList(), messageId, "mock/mock/account0");
    }

    QTRY_COMPARE(messageReadSpy.count(), messageIds.count());
    QStringList receivedIds;
    for (int i = 0; i < messageReadSpy.count(); ++i) {
        receivedIds << messageReadSpy[i][0].toString();
    }

    qSort(receivedIds);
    qSort(messageIds);
    QCOMPARE(receivedIds, messageIds);
}

QTEST_MAIN(ChatManagerTest)
#include "ChatManagerTest.moc"
