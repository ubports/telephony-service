/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include <QDateTime>
#include <QContact>
#include <QContactAvatar>
#include <QContactPhoneNumber>
#include "contactmodel.h"
#include "contactentry.h"
#include "conversationfeedmodel.h"
#include "conversationfeeditem.h"

class ConversationFeedModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testClear();
    void testRowCount();
    void testAddItem();
    void testRemoveItem();
    void testIndexFromEntry();
    void testEntryFromIndex();
    void testData();
    void testDataChanged();
    void testMatchesSearch_data();
    void testMatchesSearch();
    void testContactInfo();
    void testCheckNonStandardNumbers_data();
    void testCheckNonStandardNumbers();
    void testOnContactAdded();
    void testOnContactChanged();
    void testOnContactRemoved();

private:
    QList<ConversationFeedItem*> populateWithItems(int count);
    ConversationFeedModel *feedModel;
};


QList<ConversationFeedItem*> ConversationFeedModelTest::populateWithItems(int count)
{
    QList<ConversationFeedItem*> addedItems;

    QString alias("ContactAlias%1");
    QString avatar("/a/file/somewhere/for/contact/%1");
    QString id("ContactId%1");
    bool incoming = true;
    bool newItem = true;
    QDateTime timestamp = QDateTime::currentDateTime();

    for (int i = 0; i < count; ++i) {
        ConversationFeedItem *item = new ConversationFeedItem(feedModel);
        item->setContactAlias(alias.arg(i));
        item->setContactAvatar(avatar.arg(i));
        // leave some contacts without a contactId
        if (newItem) {
            item->setContactId(id.arg(i));
        }
        item->setIncoming(incoming);
        incoming = !incoming;
        item->setNewItem(newItem);
        newItem = !newItem;
        timestamp = timestamp.addDays(qrand() % 10);
        item->setTimestamp(timestamp);

        feedModel->addItem(item);
        addedItems.append(item);
    }

    return addedItems;
}

void ConversationFeedModelTest::initTestCase()
{
    // call the contact model with the memory backend
    ContactModel::instance("memory");
    feedModel = new ConversationFeedModel(this);
    qsrand(QTime::currentTime().msec());
}

void ConversationFeedModelTest::cleanupTestCase()
{
    delete feedModel;
}

void ConversationFeedModelTest::testClear()
{
    populateWithItems(50);

    feedModel->clear();
    QCOMPARE(feedModel->rowCount(), 0);
}


void ConversationFeedModelTest::testRowCount()
{
    int count = 10;

    populateWithItems(count);

    QCOMPARE(feedModel->rowCount(), count);

    // for any given valid index, the rowCount needs to be zero
    for (int i = 0; i < count; ++i) {
        QModelIndex index = feedModel->index(i);
        QCOMPARE(feedModel->rowCount(index), 0);
    }

    feedModel->clear();
}

void ConversationFeedModelTest::testAddItem()
{
    int count = 13;
    QSignalSpy aboutToBeInsertedSignalSpy(feedModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    QSignalSpy insertedSignalSpy(feedModel, SIGNAL(rowsInserted(QModelIndex,int,int)));

    for (int i = 0; i < count; ++i) {
        ConversationFeedItem *item = new ConversationFeedItem(feedModel);
        feedModel->addItem(item);

        // check if the signals were emitted once per item added
        QCOMPARE(aboutToBeInsertedSignalSpy.count(), 1);
        QCOMPARE(insertedSignalSpy.count(), 1);

        // check that the row number passed is corect
        QCOMPARE(aboutToBeInsertedSignalSpy[0][1].toInt(), i);
        QCOMPARE(aboutToBeInsertedSignalSpy[0][2].toInt(), i);

        QCOMPARE(insertedSignalSpy[0][1].toInt(), i);
        QCOMPARE(insertedSignalSpy[0][2].toInt(), i);

        aboutToBeInsertedSignalSpy.clear();
        insertedSignalSpy.clear();
    }

    feedModel->clear();
}

void ConversationFeedModelTest::testRemoveItem()
{
    QSignalSpy aboutToBeRemovedSignalSpy(feedModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    QSignalSpy removedSignalSpy(feedModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    // insert a few items
    int count = 20;

    QList<ConversationFeedItem*> addedItems = populateWithItems(count);

    // remove five items at random
    for (int i = 0; i < 5; ++i) {
        int pos = qrand() % count;
        feedModel->removeItem(addedItems[pos]);

        // check if the signals were emitted once per item added
        QCOMPARE(aboutToBeRemovedSignalSpy.count(), 1);
        QCOMPARE(removedSignalSpy.count(), 1);

        // check that the row number passed is corect
        QCOMPARE(aboutToBeRemovedSignalSpy[0][1].toInt(), pos);
        QCOMPARE(aboutToBeRemovedSignalSpy[0][2].toInt(), pos);

        QCOMPARE(removedSignalSpy[0][1].toInt(), pos);
        QCOMPARE(removedSignalSpy[0][2].toInt(), pos);

        QCOMPARE(feedModel->rowCount(), --count);

        aboutToBeRemovedSignalSpy.clear();
        removedSignalSpy.clear();
        addedItems.removeAt(pos);
    }

    feedModel->clear();
}

void ConversationFeedModelTest::testIndexFromEntry()
{
    // insert some
    int count = 45;

    QList<ConversationFeedItem*> addedItems = populateWithItems(count);

    // Try to access 15 items at random
    for (int i = 0; i < 15; ++i) {
        int pos = qrand() % count;

        ConversationFeedItem *item = addedItems[pos];
        QModelIndex index  = feedModel->indexFromEntry(item);

        QVERIFY(index.isValid());
        QCOMPARE(index.row(), pos);
    }

    // passing a null or invalid entry should return an invalid QModelIndex
    QVERIFY(!feedModel->indexFromEntry(0).isValid());

    feedModel->clear();
}

void ConversationFeedModelTest::testEntryFromIndex()
{
    // insert some
    int count = 40;

    QList<ConversationFeedItem*> addedItems = populateWithItems(count);

    // Try to access 15 items at random
    for (int i = 0; i < 15; ++i) {
        int pos = qrand() % count;

        QModelIndex index = feedModel->index(pos);
        QVERIFY(index.isValid());

        ConversationFeedItem *item = feedModel->entryFromIndex(index);
        QCOMPARE(item, addedItems[pos]);
    }

    // passing an invalid index should return null
    QCOMPARE(feedModel->entryFromIndex(QModelIndex()), static_cast<ConversationFeedItem*>(0));

    feedModel->clear();
}

void ConversationFeedModelTest::testData()
{
    int count = 50;
    QList<ConversationFeedItem*> addedItems = populateWithItems(count);

    // Try to access 15 items at random
    for (int i = 0; i < 15; ++i) {
        int pos = qrand() % count;

        QModelIndex index = feedModel->index(pos);
        QVERIFY(index.isValid());

        ConversationFeedItem *item = addedItems[pos];

        QCOMPARE(feedModel->data(index, ConversationFeedModel::ContactAlias).toString(), item->contactAlias());
        QCOMPARE(feedModel->data(index, ConversationFeedModel::ContactAvatar).toUrl(), item->contactAvatar());
        QCOMPARE(feedModel->data(index, ConversationFeedModel::ContactId).toString(), item->contactId());
        QCOMPARE(feedModel->data(index, ConversationFeedModel::Incoming).toBool(), item->incoming());
        QCOMPARE(feedModel->data(index, ConversationFeedModel::NewItem).toBool(), item->newItem());
        QCOMPARE(feedModel->data(index, ConversationFeedModel::Timestamp).toDateTime(), item->timestamp());
        QCOMPARE(feedModel->data(index, ConversationFeedModel::Date).toDate(), item->timestamp().date());
        QCOMPARE(feedModel->data(index, ConversationFeedModel::ItemType).toString(), feedModel->itemType(index));
        QCOMPARE(feedModel->data(index, ConversationFeedModel::GroupingProperty).toString(), QString(item->contactId().isEmpty() ? "phoneNumber" : "contactId"));

        ConversationFeedItem *returnedItem = qobject_cast<ConversationFeedItem*>(feedModel->data(index, ConversationFeedModel::FeedItem).value<QObject*>());
        QCOMPARE(returnedItem, item);
    }

    // check if accessing data() for an invalid index returns a null QVariant
    QVERIFY(feedModel->data(QModelIndex(), ConversationFeedModel::ContactAlias).isNull());

    feedModel->clear();
}

void ConversationFeedModelTest::testDataChanged()
{
    int count = 30;
    QList<ConversationFeedItem*> addedItems = populateWithItems(count);
    QSignalSpy signalSpy(feedModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)));

    // Try to access 15 items at random
    for (int i = 0; i < 15; ++i) {
        int pos = qrand() % count;

        // change the properties of the item and check if the dataChanged signal is emitted
        ConversationFeedItem *item = addedItems[pos];

        int emitCount = 0;
        item->setContactId("AnotherContactId");
        QCOMPARE(signalSpy.count(), ++emitCount);
        QCOMPARE(signalSpy[emitCount-1][0].value<QModelIndex>().row(), pos);
        QCOMPARE(signalSpy[emitCount-1][1].value<QModelIndex>().row(), pos);

        item->setContactAlias("AnotherContactAlias");
        QCOMPARE(signalSpy.count(), ++emitCount);
        QCOMPARE(signalSpy[emitCount-1][0].value<QModelIndex>().row(), pos);
        QCOMPARE(signalSpy[emitCount-1][1].value<QModelIndex>().row(), pos);

        item->setContactAvatar(QUrl("/AnotherContactAvatar"));
        QCOMPARE(signalSpy.count(), ++emitCount);
        QCOMPARE(signalSpy[emitCount-1][0].value<QModelIndex>().row(), pos);
        QCOMPARE(signalSpy[emitCount-1][1].value<QModelIndex>().row(), pos);

        item->setIncoming(true);
        QCOMPARE(signalSpy.count(), ++emitCount);
        QCOMPARE(signalSpy[emitCount-1][0].value<QModelIndex>().row(), pos);
        QCOMPARE(signalSpy[emitCount-1][1].value<QModelIndex>().row(), pos);

        item->setNewItem(true);
        QCOMPARE(signalSpy.count(), ++emitCount);
        QCOMPARE(signalSpy[emitCount-1][0].value<QModelIndex>().row(), pos);
        QCOMPARE(signalSpy[emitCount-1][1].value<QModelIndex>().row(), pos);

        item->setTimestamp(QDateTime::currentDateTime());
        QCOMPARE(signalSpy.count(), ++emitCount);
        QCOMPARE(signalSpy[emitCount-1][0].value<QModelIndex>().row(), pos);
        QCOMPARE(signalSpy[emitCount-1][1].value<QModelIndex>().row(), pos);

        signalSpy.clear();
    }

    feedModel->clear();
}

void ConversationFeedModelTest::testMatchesSearch_data()
{
    QTest::addColumn<QString>("contactAlias");
    QTest::addColumn<QString>("phoneNumber");
    QTest::addColumn<QString>("searchTerm");
    QTest::addColumn<int>("rowNumber");
    QTest::addColumn<bool>("result");

    QTest::newRow("exact alias match") << "Contact Alias" << "12345" << "Contact Alias" << 0 << true;
    QTest::newRow("parcial alias match") << "One Contact Alias" << "12345" << "Contact" << 0 << true;
    QTest::newRow("case insensitive match") << "OnE CoNtAcT" << "12345" << "One Contact" << 0 << true;
    QTest::newRow("exact phone number match") << "One Contact" << "6584945" << "6584945" << 0 << true;
    QTest::newRow("prefixed phone number match") << "Another Contact" << "+1 22 3456 7890" << "34567890" << 0 << true;
    QTest::newRow("no match") << "Yet Another Contact" << "12345" << "no match" << 0 << false;
    QTest::newRow("invalid index") << "Contact" << "12345" << "Contact" << -1 << false;
}

void ConversationFeedModelTest::testMatchesSearch()
{
    QFETCH(QString, contactAlias);
    QFETCH(QString, phoneNumber);
    QFETCH(QString, searchTerm);
    QFETCH(int, rowNumber);
    QFETCH(bool, result);

    ConversationFeedItem *item = populateWithItems(1).first();
    item->setContactAlias(contactAlias);
    item->setPhoneNumber(phoneNumber);
    QCOMPARE(feedModel->matchesSearch(searchTerm, feedModel->index(rowNumber)), result);
    feedModel->clear();
}

void ConversationFeedModelTest::testContactInfo()
{
    // create a fake contact to fill the data
    QContact contact;
    QContactDisplayLabel labelDetail;
    labelDetail.setLabel("Fake Contact");
    QVERIFY(contact.saveDetail(&labelDetail));

    QContactAvatar avatarDetail;
    avatarDetail.setImageUrl(QUrl::fromLocalFile("/path/to/some/image.png"));
    QVERIFY(contact.saveDetail(&avatarDetail));

    ContactEntry entry(contact);

    // fill the contact info and check if it is ok
    ConversationFeedItem *item = populateWithItems(1).first();
    feedModel->fillContactInfo(item, &entry);

    QCOMPARE(item->contactId(), entry.id().toString());
    QCOMPARE(item->contactAvatar(), entry.avatar());
    QCOMPARE(item->contactAlias(), entry.displayLabel());

    // now clear the contact info and check it was indeed cleared
    feedModel->clearContactInfo(item);
    QVERIFY(item->contactId().isEmpty());
    QVERIFY(item->contactAvatar().toString().isEmpty());
    QVERIFY(item->contactAlias().isEmpty());
}

void ConversationFeedModelTest::testCheckNonStandardNumbers_data()
{
    QTest::addColumn<QString>("phoneNumber");
    QTest::addColumn<QString>("modifiedPhoneNumber");
    QTest::addColumn<bool>("aliasIsEmpty");
    QTest::addColumn<bool>("result");

    QTest::newRow("private number") << "-2" << "-" << false << true;
    QTest::newRow("unknown number -1") << "-1" << "-" << false << true;
    QTest::newRow("unknown number #") << "#" << "-" << false << true;
    QTest::newRow("valid number") << "12345" << "12345" << true << false;
}

void ConversationFeedModelTest::testCheckNonStandardNumbers()
{
    QFETCH(QString, phoneNumber);
    QFETCH(QString, modifiedPhoneNumber);
    QFETCH(bool, aliasIsEmpty);
    QFETCH(bool, result);

    ConversationFeedItem *item = new ConversationFeedItem(this);
    item->setPhoneNumber(phoneNumber);
    QCOMPARE(feedModel->checkNonStandardNumbers(item), result);
    QCOMPARE(item->contactAlias().isEmpty(), aliasIsEmpty);
    QCOMPARE(item->phoneNumber(), modifiedPhoneNumber);
    delete item;

    // check if it returns false for a null item
    QCOMPARE(feedModel->checkNonStandardNumbers(0), false);
}

void ConversationFeedModelTest::testOnContactAdded()
{
    QList<ConversationFeedItem*> addedItems = populateWithItems(5);

    // change one position at random that will match the contact
    int pos = qrand() % 5;
    ConversationFeedItem *item = addedItems[pos];
    item->setPhoneNumber("55554444");

    // create a fake contact to fill the data
    QContact contact;
    QContactDisplayLabel labelDetail;
    labelDetail.setLabel("Fake Contact");
    QVERIFY(contact.saveDetail(&labelDetail));

    QContactPhoneNumber phoneDetail;
    phoneDetail.setNumber(item->phoneNumber());
    QVERIFY(contact.saveDetail(&phoneDetail));

    ContactModel::instance()->contactManager()->saveContact(&contact);
    ContactEntry entry(contact);
    QCOMPARE(item->contactId(), entry.idString());
    QCOMPARE(item->contactAlias(), entry.displayLabel());

    feedModel->clear();
    ContactModel::instance()->contactManager()->removeContact(contact.id());
}

void ConversationFeedModelTest::testOnContactChanged()
{
    // create a contact in the model
    QContact contact;
    QContactDisplayLabel labelDetail;
    labelDetail.setLabel("Fake Contact");
    QVERIFY(contact.saveDetail(&labelDetail));

    ContactModel::instance()->contactManager()->saveContact(&contact);
    ContactEntry contactEntry(contact);

    ConversationFeedItem *item1 = new ConversationFeedItem(feedModel);
    ConversationFeedItem *item2 = new ConversationFeedItem(feedModel);
    item1->setPhoneNumber("55554444");
    item2->setPhoneNumber("44445555");

    feedModel->addItem(item1);
    feedModel->addItem(item2);

    // now modify the contact to have a phone matching item1
    QContactPhoneNumber phoneDetail;
    phoneDetail.setNumber("55554444");
    QVERIFY(contact.saveDetail(&phoneDetail));

    ContactModel::instance()->contactManager()->saveContact(&contact);

    // and verify the item was properly updated
    QCOMPARE(item1->contactId(), contactEntry.idString());
    QCOMPARE(item1->contactAlias(), contactEntry.displayLabel());
    QVERIFY(item2->contactId().isEmpty());
    QVERIFY(item2->contactAlias().isEmpty());

    // now change the phone number and check that items are properly updated
    phoneDetail.setNumber("44445555");
    QVERIFY(contact.saveDetail(&phoneDetail));

    ContactModel::instance()->contactManager()->saveContact(&contact);

    QCOMPARE(item2->contactId(), contactEntry.idString());
    QCOMPARE(item2->contactAlias(), contactEntry.displayLabel());
    QVERIFY(item1->contactId().isEmpty());
    QVERIFY(item1->contactAlias().isEmpty());

    feedModel->clear();
    ContactModel::instance()->contactManager()->removeContact(contact.id());
}

void ConversationFeedModelTest::testOnContactRemoved()
{
    ConversationFeedItem *item = new ConversationFeedItem(feedModel);
    item->setPhoneNumber("55554444");
    feedModel->addItem(item);

    // create a contact in the model
    QContact contact;
    QContactDisplayLabel labelDetail;
    labelDetail.setLabel("Fake Contact");
    QVERIFY(contact.saveDetail(&labelDetail));

    QContactPhoneNumber phoneDetail;
    phoneDetail.setNumber("55554444");
    QVERIFY(contact.saveDetail(&phoneDetail));

    ContactModel::instance()->contactManager()->saveContact(&contact);
    ContactEntry contactEntry(contact);

    QCOMPARE(item->contactAlias(), contactEntry.displayLabel());
    QCOMPARE(item->contactId(), contactEntry.idString());

    // now remove the contact to see if the item is cleared
    ContactModel::instance()->contactManager()->removeContact(contact.id());

    QVERIFY(item->contactAlias().isEmpty());
    QVERIFY(item->contactId().isEmpty());

    feedModel->clear();
}


QTEST_MAIN(ConversationFeedModelTest)
#include "ConversationFeedModelTest.moc"
