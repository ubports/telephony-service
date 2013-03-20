/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include <QContact>
#include <QContactPhoneNumber>
#include "messagelogmodel.h"
#include "contactmodel.h"
#include "contactentry.h"

class MessageLogModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testMessageLogEntryMessage();
    void testMessageLogEntryMessageId();
    void testMessageLogEntryDate();
    void testItemType();
    void testMatchesSearch_data();
    void testMatchesSearch();
    void testAppendMessage_data();
    void testAppendMessage();
    void testMessageReadSignal();
    void testOnMessageSent();
    void testOnMessageReceived();

private:
    MessageLogModel *messageModel;
};

void MessageLogModelTest::initTestCase()
{
    // instanciate the contact model using the memory backend
    ContactModel::instance("memory");

    messageModel = new MessageLogModel(this);
}

void MessageLogModelTest::cleanupTestCase()
{
    delete messageModel;
}

void MessageLogModelTest::testMessageLogEntryMessage()
{
    MessageLogEntry *entry = new MessageLogEntry(this);

    QSignalSpy signalSpy(entry, SIGNAL(messageChanged()));

    QString message("One test message");
    entry->setMessage(message);

    QCOMPARE(entry->message(), message);
    QCOMPARE(signalSpy.count(), 1);

    delete entry;
}

void MessageLogModelTest::testMessageLogEntryMessageId()
{
    MessageLogEntry *entry = new MessageLogEntry(this);

    QSignalSpy signalSpy(entry, SIGNAL(messageIdChanged()));

    QString messageId("the_message_id");
    entry->setMessageId(messageId);

    QCOMPARE(entry->messageId(), messageId);
    QCOMPARE(signalSpy.count(), 1);

    delete entry;
}

void MessageLogModelTest::testMessageLogEntryDate()
{
    MessageLogEntry *entry = new MessageLogEntry(this);

    QDate date(1989, 11, 20);
    entry->setTimestamp(QDateTime(date, QTime(0,0,0)));
    QCOMPARE(entry->date(), date.toString(Qt::DefaultLocaleLongDate));

    delete entry;
}

void MessageLogModelTest::testItemType()
{
    MessageLogEntry *entry = new MessageLogEntry(messageModel);
    messageModel->addItem(entry);

    // test item type on a valid index
    QCOMPARE(messageModel->itemType(messageModel->index(0)), QString("message"));

    // and an invalid index
    QCOMPARE(messageModel->itemType(QModelIndex()), QString("message"));

    messageModel->clear();
}

void MessageLogModelTest::testMatchesSearch_data()
{
    QTest::addColumn<QString>("message");
    QTest::addColumn<QString>("searchTerm");
    QTest::addColumn<bool>("result");

    QTest::newRow("exact match") << "one message test" << "one message test" << true;
    QTest::newRow("parcial all lowercase match") << "another way to test a message" << "way" << true;
    QTest::newRow("case insensitive match") << "sImPlE mEsSaGe" << "MeSsAgE" << true;
    QTest::newRow("no match") << "This should not match" << "with that" << false;
}

void MessageLogModelTest::testMatchesSearch()
{
    QFETCH(QString, message);
    QFETCH(QString, searchTerm);
    QFETCH(bool, result);

    MessageLogEntry *entry = new MessageLogEntry(messageModel);
    entry->setMessage(message);
    messageModel->addItem(entry);

    QCOMPARE(messageModel->matchesSearch(searchTerm, messageModel->index(0)), result);

    // test if matchesSearch returns false for an invalid index
    QCOMPARE(messageModel->matchesSearch(searchTerm, QModelIndex()), false);

    messageModel->clear();
}

void MessageLogModelTest::testAppendMessage_data()
{
    QTest::addColumn<QString>("phoneNumber");
    QTest::addColumn<bool>("incoming");
    QTest::addColumn<QDateTime>("timestamp");
    QTest::addColumn<QString>("messageId");
    QTest::addColumn<QString>("message");
    QTest::addColumn<bool>("newEvent");
    QTest::addColumn<bool>("hasContact");

    QTest::newRow("unread incoming message without contact")
            << "1234 1234" << true << QDateTime(QDate(2011, 10, 15), QTime(18, 42, 26, 452)) << "message1" << "One message" << true << false;
    QTest::newRow("unread incoming message with contact")
            << "3456 7890" << true << QDateTime(QDate(2012, 8, 21), QTime(15, 10, 30, 123)) << "message2" << "Another message" << true << true;
    QTest::newRow("incoming message without contact")
            << "1111 2222" << true << QDateTime(QDate(2011, 10, 15), QTime(23, 30, 2, 765)) << "message3" << "One more message" << false << false;
    QTest::newRow("incoming message with contact")
            << "3333 4444" << true << QDateTime(QDate(2013, 2, 28), QTime(8, 5, 54, 999)) << "message4" << "Hi there" << false << true;
    QTest::newRow("outgoing message without contact")
            << "9999 0000" << false << QDateTime(QDate(2006, 6, 6), QTime(6, 6, 6, 6)) << "message5" << "Just checked your message, thanks" << false << false;
    QTest::newRow("outgoing message with contact")
            << "9999 8888" << false << QDateTime(QDate(2007, 7, 7), QTime(23, 40, 40, 888)) << "message6" << "are you still there?" << false << true;
}

void MessageLogModelTest::testAppendMessage()
{
    QFETCH(QString, phoneNumber);
    QFETCH(bool, incoming);
    QFETCH(QDateTime, timestamp);
    QFETCH(QString, messageId);
    QFETCH(QString, message);
    QFETCH(bool, newEvent);
    QFETCH(bool, hasContact);

    QContact contact;
    if (hasContact) {
        // create a fake contact matching the phone number
        QContactDisplayLabel labelDetail;
        labelDetail.setLabel(QString("Fake Contact with number %1").arg(phoneNumber));
        QVERIFY(contact.saveDetail(&labelDetail));

        QContactPhoneNumber phoneDetail;
        phoneDetail.setNumber(phoneNumber);
        QVERIFY(contact.saveDetail(&phoneDetail));

        ContactModel::instance()->contactManager()->saveContact(&contact);
    }

    QSignalSpy signalSpy(messageModel, SIGNAL(rowsInserted(QModelIndex,int,int)));
    messageModel->appendMessage(phoneNumber, message, incoming, timestamp, messageId, newEvent);

    QCOMPARE(signalSpy.count(), 1);

    // get the item
    MessageLogEntry *entry = qobject_cast<MessageLogEntry*>(messageModel->entryFromIndex(messageModel->index(0)));
    QCOMPARE(entry->phoneNumber(), phoneNumber);
    QCOMPARE(entry->incoming(), incoming);
    QCOMPARE(entry->timestamp(), timestamp);
    QCOMPARE(entry->messageId(), messageId);
    QCOMPARE(entry->message(), message);
    QCOMPARE(entry->newItem(), newEvent);

    if (hasContact) {
        ContactEntry contactEntry(contact);
        QCOMPARE(entry->contactAlias(), contactEntry.displayLabel());
        QCOMPARE(entry->contactId(), contactEntry.idString());

        // remove the contact
        ContactModel::instance()->contactManager()->removeContact(contact.id());
    } else {
        QVERIFY(entry->contactId().isEmpty());
        QCOMPARE(entry->contactAlias(), phoneNumber);
    }

    messageModel->clear();
}

void MessageLogModelTest::testMessageReadSignal()
{
    messageModel->appendMessage("33333333", "test message", true, QDateTime::currentDateTime(), "messageId", true);
    MessageLogEntry *item = qobject_cast<MessageLogEntry*>(messageModel->entryFromIndex(messageModel->index(0)));
    QVERIFY(item);

    QSignalSpy signalSpy(messageModel, SIGNAL(messageRead(QString,QString)));

    item->setNewItem(false);

    QCOMPARE(signalSpy.count(), 1);
    QCOMPARE(signalSpy[0][0].toString(), item->phoneNumber());
    QCOMPARE(signalSpy[0][1].toString(), item->messageId());

    messageModel->clear();
}

void MessageLogModelTest::testOnMessageSent()
{
    QSignalSpy signalSpy(messageModel, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QString phoneNumber("12345");
    QString message("I wrote a message");

    messageModel->onMessageSent(phoneNumber, message);

    QCOMPARE(signalSpy.count(), 1);

    MessageLogEntry *entry = qobject_cast<MessageLogEntry*>(messageModel->entryFromIndex(messageModel->index(0)));
    QCOMPARE(entry->phoneNumber(), phoneNumber);
    QCOMPARE(entry->message(), message);

    messageModel->clear();
}

void MessageLogModelTest::testOnMessageReceived()
{
    QSignalSpy signalSpy(messageModel, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QString phoneNumber("12345");
    QString message("I wrote a message");
    QString messageId("firstMessageId");
    bool unread = true;

    // test if a received message is added correctly
    messageModel->onMessageReceived(phoneNumber, message, QDateTime::currentDateTime(), messageId, unread);

    QCOMPARE(signalSpy.count(), 1);

    MessageLogEntry *entry = qobject_cast<MessageLogEntry*>(messageModel->entryFromIndex(messageModel->index(0)));
    QCOMPARE(entry->phoneNumber(), phoneNumber);
    QCOMPARE(entry->message(), message);
    QCOMPARE(entry->messageId(), messageId);
    QCOMPARE(entry->newItem(), unread);

    // now try to add the same item again, it should not be added
    messageModel->onMessageReceived(phoneNumber, message, QDateTime::currentDateTime(), messageId, unread);
    QCOMPARE(signalSpy.count(), 1);

    messageModel->clear();
}

QTEST_MAIN(MessageLogModelTest)
#include "MessageLogModelTest.moc"
