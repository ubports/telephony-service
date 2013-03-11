/*
 * Copyright (C) 2012 Canonical, Ltd.
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
#include "conversationaggregatormodel.h"
#include "conversationfeeditem.h"
#include "conversationfeedmodel.h"

class ConversationAggregatorModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testAddFeedModel();
    void testRemoveFeedModel();
    void testRowCount();
    void testIndex();
    void testMapToSource();
    void testMapFromSource();
    void testData();
    void testDataChanged();
    void testRowsInserted();
    void testRowsRemoved();

private:
    QList<ConversationFeedModel*> feedModels;
    ConversationAggregatorModel *aggregatorModel;
};

void ConversationAggregatorModelTest::initTestCase()
{
    qsrand(QTime::currentTime().msec());

    aggregatorModel = new ConversationAggregatorModel(this);

    // use a few models for the basic tests
    for (int i = 0; i < 5; ++i) {
        ConversationFeedModel *feedModel = new ConversationFeedModel(aggregatorModel);

        // insert 3 empty items in each model
        for (int j = 0; j < 15; ++j) {
            ConversationFeedItem *item = new ConversationFeedItem(feedModel);
            feedModel->addItem(item);
        }

        feedModels.append(feedModel);
        aggregatorModel->addFeedModel(feedModel);
    }
}

void ConversationAggregatorModelTest::cleanupTestCase()
{
    delete aggregatorModel;
}

void ConversationAggregatorModelTest::testAddFeedModel()
{
    // the aggregator model does not expose the internal models, so in order to test
    // the addition of a feed model, we check if the rowsInserted signal is emitted
    QSignalSpy signalSpy(aggregatorModel, SIGNAL(rowsInserted(QModelIndex,int,int)));

    ConversationFeedModel *feedModel = new ConversationFeedModel(aggregatorModel);
    ConversationFeedItem *item = new ConversationFeedItem(feedModel);
    feedModel->addItem(item);

    feedModels.append(feedModel);
    aggregatorModel->addFeedModel(feedModel);

    QCOMPARE(signalSpy.count(), 1);
}

void ConversationAggregatorModelTest::testRemoveFeedModel()
{
    // the aggregator model does not expose the internal models, so in order to test
    // the removal of a feed model, we check if the rowsRemoved signal is emitted
    QSignalSpy signalSpy(aggregatorModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    // remove one random model
    int pos = qrand() % feedModels.count();
    ConversationFeedModel *feedModel = feedModels.takeAt(pos);
    aggregatorModel->removeFeedModel(feedModel);

    QCOMPARE(signalSpy.count(), 1);
}

void ConversationAggregatorModelTest::testRowCount()
{
    int rowCount = 0;
    Q_FOREACH(ConversationFeedModel *model, feedModels) {
        rowCount += model->rowCount();
    }
    QCOMPARE(aggregatorModel->rowCount(), rowCount);

    // insert a few random items and check the rowCount is updated correctly
    for (int i = 0; i < 10; ++i) {
        ConversationFeedModel *model = feedModels[qrand() % feedModels.count()];
        ConversationFeedItem *item = new ConversationFeedItem(model);
        model->addItem(item);
        rowCount++;
    }
    QCOMPARE(aggregatorModel->rowCount(), rowCount);
}

void ConversationAggregatorModelTest::testIndex()
{
    // in order to make sure the returned data is correct, we calculate the offsets locally
    // here in the test and compare against the results returned by the model
    int offset = 0;
    Q_FOREACH(ConversationFeedModel *model, feedModels) {
        // get some random rows from the model
        for (int i = 0; i < 10; ++i) {
            int sourcePos = qrand() % model->rowCount();
            int destinationPos = sourcePos + offset;

            QModelIndex index = aggregatorModel->index(destinationPos);
            QVERIFY(index.isValid());
            QCOMPARE(index.internalPointer(), (void*)model);
            QCOMPARE(index.row(), destinationPos);
            QCOMPARE(index.column(), 0);
            QVERIFY(!index.parent().isValid());
        }
        offset += model->rowCount();
    }

    // check that no child items are returned
    QModelIndex invalidIndex = aggregatorModel->index(0, 0, aggregatorModel->index(0));
    QVERIFY(!invalidIndex.isValid());
}

void ConversationAggregatorModelTest::testMapToSource()
{
    // in order to make sure the returned data is correct, we calculate the offsets locally
    // here in the test and compare against the results returned by the model
    int offset = 0;
    Q_FOREACH(ConversationFeedModel *model, feedModels) {
        // get some random rows from the model
        for (int i = 0; i < 10; ++i) {
            int sourcePos = qrand() % model->rowCount();
            int destinationPos = sourcePos + offset;

            QModelIndex index = aggregatorModel->index(destinationPos);
            QVERIFY(index.isValid());

            QModelIndex sourceIndex = aggregatorModel->mapToSource(index);
            QVERIFY(sourceIndex.isValid());
            QCOMPARE(sourceIndex.row(), sourcePos);
            QCOMPARE(sourceIndex.model(), model);
        }
        offset += model->rowCount();
    }
}

void ConversationAggregatorModelTest::testMapFromSource()
{
    // in order to make sure the returned data is correct, we calculate the offsets locally
    // here in the test and compare against the results returned by the model
    int offset = 0;
    Q_FOREACH(ConversationFeedModel *model, feedModels) {
        // get some random rows from the model
        for (int i = 0; i < 10; ++i) {
            int sourcePos = qrand() % model->rowCount();
            int destinationPos = sourcePos + offset;

            QModelIndex sourceIndex = model->index(sourcePos);
            QVERIFY(sourceIndex.isValid());

            QModelIndex index = aggregatorModel->mapFromSource(sourceIndex);
            QVERIFY(index.isValid());
            QCOMPARE(index.row(), destinationPos);
            QCOMPARE(index.model(), aggregatorModel);
        }
        offset += model->rowCount();
    }
}

void ConversationAggregatorModelTest::testData()
{
    // there is no need to test all data roles here, but at least the data returned needs to be correct.

    // pick a model at random
    ConversationFeedModel *model = feedModels[qrand() % feedModels.count()];

    // insert an item with some values filled
    QString id("ContactId");
    QString alias("ContactAlias");
    ConversationFeedItem *item = new ConversationFeedItem(model);
    item->setContactId(id);
    item->setContactAlias(alias);
    model->addItem(item);

    QModelIndex sourceIndex = model->indexFromEntry(item);
    QModelIndex index = aggregatorModel->mapFromSource(sourceIndex);

    QCOMPARE(aggregatorModel->data(index, ConversationFeedModel::ContactId).toString(), item->contactId());
    QCOMPARE(aggregatorModel->data(index, ConversationFeedModel::ContactAlias).toString(), item->contactAlias());

    ConversationFeedItem *returnedItem = qobject_cast<ConversationFeedItem*>(aggregatorModel->data(index, ConversationFeedModel::FeedItem).value<QObject*>());
    QCOMPARE(returnedItem, item);
}

void ConversationAggregatorModelTest::testDataChanged()
{
    QSignalSpy signalSpy(aggregatorModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)));

    // check if the data changed signal is being propagated correctly by the aggregator
    for (int i = 0; i < 15; ++i) {
        ConversationFeedModel *model = feedModels[qrand() % feedModels.count()];
        int row = qrand() % model->rowCount();
        QModelIndex sourceIndex = model->index(row);
        QModelIndex index = aggregatorModel->mapFromSource(sourceIndex);
        ConversationFeedItem *item = model->entryFromIndex(sourceIndex);
        item->setContactAlias(QString("ContactAlias-%1").arg(row));

        QCOMPARE(signalSpy.count(), 1);
        QCOMPARE(signalSpy[0][0].value<QModelIndex>(), index);
        QCOMPARE(signalSpy[0][1].value<QModelIndex>(), index);

        signalSpy.clear();
    }
}

void ConversationAggregatorModelTest::testRowsInserted()
{
    QSignalSpy aboutToBeInsertedSpy(aggregatorModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    QSignalSpy insertedSpy(aggregatorModel, SIGNAL(rowsInserted(QModelIndex,int,int)));

    // in order to make sure the returned data is correct, we calculate the offsets locally
    // here in the test and compare against the results returned by the model
    int offset = 0;
    Q_FOREACH(ConversationFeedModel *model, feedModels) {
        // insert a random number of items in each model
        int count = qrand() % 10;
        for (int i = 0; i < count; ++i) {
            int row = model->rowCount() + offset;

            ConversationFeedItem *item = new ConversationFeedItem(model);
            model->addItem(item);

            QCOMPARE(aboutToBeInsertedSpy.count(), 1);
            QCOMPARE(insertedSpy.count(), 1);

            QCOMPARE(aboutToBeInsertedSpy[0][1].toInt(), row);
            QCOMPARE(aboutToBeInsertedSpy[0][2].toInt(), row);

            QCOMPARE(insertedSpy[0][1].toInt(), row);
            QCOMPARE(insertedSpy[0][1].toInt(), row);

            aboutToBeInsertedSpy.clear();
            insertedSpy.clear();
        }
        offset += model->rowCount();
    }
}

void ConversationAggregatorModelTest::testRowsRemoved()
{
    QSignalSpy aboutToBeRemovedSpy(aggregatorModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    QSignalSpy removedSpy(aggregatorModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    // in order to make sure the returned data is correct, we calculate the offsets locally
    // here in the test and compare against the results returned by the model
    int offset = 0;
    Q_FOREACH(ConversationFeedModel *model, feedModels) {
        // remove a few items from each model
        for (int i = 0; i < 5; ++i) {
            if (model->rowCount() == 0) {
                break;
            }

            int sourceRow = qrand() % model->rowCount();
            int destinationRow = sourceRow + offset;

            ConversationFeedItem *item = model->entryFromIndex(model->index(sourceRow));
            model->removeItem(item);

            QCOMPARE(aboutToBeRemovedSpy.count(), 1);
            QCOMPARE(removedSpy.count(), 1);

            QCOMPARE(aboutToBeRemovedSpy[0][1].toInt(), destinationRow);
            QCOMPARE(aboutToBeRemovedSpy[0][2].toInt(), destinationRow);

            QCOMPARE(removedSpy[0][1].toInt(), destinationRow);
            QCOMPARE(removedSpy[0][2].toInt(), destinationRow);

            aboutToBeRemovedSpy.clear();
            removedSpy.clear();
        }
        offset += model->rowCount();
    }
}

QTEST_MAIN(ConversationAggregatorModelTest)
#include "ConversationAggregatorModelTest.moc"
