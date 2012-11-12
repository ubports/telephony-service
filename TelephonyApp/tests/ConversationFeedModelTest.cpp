/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
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
    QDateTime timestamp = QDateTime::currentDateTime();

    for (int i = 0; i < count; ++i) {
        ConversationFeedItem *item = new ConversationFeedItem(feedModel);
        item->setContactAlias(alias.arg(i));
        item->setContactAvatar(avatar.arg(i));
        item->setContactId(id.arg(i));
        item->setIncoming(incoming);
        incoming = !incoming;
        timestamp = timestamp.addDays(qrand() % 10);
        item->setTimestamp(timestamp);

        feedModel->addItem(item);
        addedItems.append(item);
    }

    return addedItems;
}

void ConversationFeedModelTest::initTestCase()
{
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
        QCOMPARE(feedModel->data(index, ConversationFeedModel::Timestamp).toDateTime(), item->timestamp());
        QCOMPARE(feedModel->data(index, ConversationFeedModel::ItemType).toString(), feedModel->itemType(index));

        ConversationFeedItem *returnedItem = qobject_cast<ConversationFeedItem*>(feedModel->data(index, ConversationFeedModel::FeedItem).value<QObject*>());
        QCOMPARE(returnedItem, item);
    }

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

        item->setTimestamp(QDateTime::currentDateTime());
        QCOMPARE(signalSpy.count(), ++emitCount);
        QCOMPARE(signalSpy[emitCount-1][0].value<QModelIndex>().row(), pos);
        QCOMPARE(signalSpy[emitCount-1][1].value<QModelIndex>().row(), pos);

        signalSpy.clear();
    }

    feedModel->clear();
}

QTEST_MAIN(ConversationFeedModelTest)
#include "ConversationFeedModelTest.moc"
