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
#include "calllogmodel.h"
#include "contactmodel.h"
#include "contactentry.h"

class CallLogModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testCallLogEntryDuration();
    void testCallLogEntryMissed();
    void testItemType();
    void testAddCallEvent_data();
    void testAddCallEvent();
private:
    CallLogModel *callModel;
};

void CallLogModelTest::initTestCase()
{
    // instanciate the contact model using the memory backend
    ContactModel::instance("memory");

    callModel = new CallLogModel(this);
}

void CallLogModelTest::cleanupTestCase()
{
    delete callModel;
}

void CallLogModelTest::testCallLogEntryDuration()
{
    CallLogEntry *entry = new CallLogEntry(this);

    QSignalSpy signalSpy(entry, SIGNAL(durationChanged()));

    QTime duration(2,20,30);
    entry->setDuration(duration);

    QCOMPARE(entry->duration(), duration);
    QCOMPARE(signalSpy.count(), 1);

    delete entry;
}

void CallLogModelTest::testCallLogEntryMissed()
{
    CallLogEntry *entry = new CallLogEntry(this);

    QSignalSpy signalSpy(entry, SIGNAL(missedChanged()));

    bool missed = false;

    // try to alternate between true and false a few times to make sure
    for (int i = 1; i < 4; ++i) {
        missed = !missed;
        entry->setMissed(missed);
        QCOMPARE(entry->missed(), missed);
        QCOMPARE(signalSpy.count(), i);
    }

    delete entry;
}

void CallLogModelTest::testItemType()
{
    CallLogEntry *entry = new CallLogEntry(callModel);
    callModel->addItem(entry);

    // test item type on a valid index
    QCOMPARE(callModel->itemType(callModel->index(0)), QString("call"));

    // and an invalid index
    QCOMPARE(callModel->itemType(QModelIndex()), QString("call"));

    callModel->clear();
}

void CallLogModelTest::testAddCallEvent_data()
{
    QTest::addColumn<QString>("phoneNumber");
    QTest::addColumn<bool>("incoming");
    QTest::addColumn<QDateTime>("timestamp");
    QTest::addColumn<QTime>("duration");
    QTest::addColumn<bool>("missed");
    QTest::addColumn<bool>("newEvent");
    QTest::addColumn<bool>("hasContact");

    QTest::newRow("unread missed incoming call without contact")
            << "1234 1234" << true << QDateTime(QDate(2011, 10, 15), QTime(18, 42, 26, 452)) << QTime(0, 0, 0) << true << true << false;
    QTest::newRow("unread missed incoming call with contact")
            << "3456 7890" << true << QDateTime(QDate(2012, 8, 21), QTime(15, 10, 30, 123)) << QTime(0, 0, 0) << true << true << true;
    QTest::newRow("missed incoming call without contact")
            << "1111 2222" << true << QDateTime(QDate(2011, 10, 15), QTime(23, 30, 2, 765)) << QTime(0, 0, 0) << true << false << false;
    QTest::newRow("missed incoming call with contact")
            << "3333 4444" << true << QDateTime(QDate(2013, 2, 28), QTime(8, 5, 54, 999)) << QTime(0, 0, 0) << true << false << true;
    QTest::newRow("unread incoming call without contact")
            << "5555 6666" << true << QDateTime(QDate(2008, 12, 14), QTime(22, 40, 55, 678)) << QTime(1, 2, 3) << false << true << false;
    QTest::newRow("unread incoming call with contact")
            << "7777 8888" << true << QDateTime(QDate(2009, 7, 30), QTime(0, 59, 11, 256)) << QTime(2, 3, 4) << false << true << true;
    QTest::newRow("incoming call without contact")
            << "9999 0000" << true << QDateTime(QDate(2010, 1, 1), QTime(10, 3, 14, 0)) << QTime(3, 4, 5) << false << false << false;
    QTest::newRow("incoming call with contact")
            << "9999 8888" << true << QDateTime(QDate(2011, 4, 3), QTime(23, 40, 40, 888)) << QTime(4, 5,6) << false << false << true;
    QTest::newRow("unread outgoing call without contact")
            << "5555 6666" << false << QDateTime(QDate(2004, 4, 4), QTime(4, 4, 4, 4)) << QTime(5, 6, 7) << false << true << false;
    QTest::newRow("unread outgoing call with contact")
            << "7777 8888" << false << QDateTime(QDate(2005, 5, 5), QTime(5, 5, 5, 5)) << QTime(6, 7, 8) << false << true << true;
    QTest::newRow("outgoing call without contact")
            << "9999 0000" << false << QDateTime(QDate(2006, 6, 6), QTime(6, 6, 6, 6)) << QTime(7, 8, 9) << false << false << false;
    QTest::newRow("outgoing call with contact")
            << "9999 8888" << false << QDateTime(QDate(2007, 7, 7), QTime(23, 40, 40, 888)) << QTime(9, 10, 11) << false << false << true;
}

void CallLogModelTest::testAddCallEvent()
{
    QFETCH(QString, phoneNumber);
    QFETCH(bool, incoming);
    QFETCH(QDateTime, timestamp);
    QFETCH(QTime, duration);
    QFETCH(bool, missed);
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

    QSignalSpy signalSpy(callModel, SIGNAL(rowsInserted(QModelIndex,int,int)));
    callModel->addCallEvent(phoneNumber, incoming, timestamp, duration, missed, newEvent);

    QCOMPARE(signalSpy.count(), 1);

    // get the item
    CallLogEntry *entry = qobject_cast<CallLogEntry*>(callModel->entryFromIndex(callModel->index(0)));
    QCOMPARE(entry->phoneNumber(), phoneNumber);
    QCOMPARE(entry->incoming(), incoming);
    QCOMPARE(entry->timestamp(), timestamp);
    QCOMPARE(entry->duration(), duration);
    QCOMPARE(entry->missed(), missed);
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

    callModel->clear();
}

QTEST_MAIN(CallLogModelTest)
#include "CallLogModelTest.moc"
