/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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

#include "conversationproxymodel.h"
#include "conversationfeeditem.h"
#include "conversationfeedmodel.h"
#include "conversationaggregatormodel.h"
#include "messagelogmodel.h"
#include "contactmodel.h"

ConversationProxyModel::ConversationProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent), mAscending(true), mGrouped(false), mShowLatestFromGroup(false)
{
    setSortRole(ConversationFeedModel::Timestamp);
    setDynamicSortFilter(true);
    updateSorting();
}

QString ConversationProxyModel::filterValue() const
{
    return mFilterValue;
}

void ConversationProxyModel::setFilterValue(const QString &value)
{
    mFilterValue = value;
    invalidateFilter();
    Q_EMIT filterValueChanged();
    // when the filter changes we also have to reset the model as the
    // ListView element seems to not behave correctly when rows
    // are changed.
    reset();
}

QString ConversationProxyModel::filterProperty() const
{
    return mFilterProperty;
}

void ConversationProxyModel::setFilterProperty(const QString &value)
{
    mFilterProperty = value;
    invalidateFilter();
    Q_EMIT filterPropertyChanged();
    // when the filter changes we also have to reset the model as the
    // ListView element seems to not behave correctly when rows
    // are changed.
    reset();
}


bool ConversationProxyModel::ascending() const
{
    return mAscending;
}

void ConversationProxyModel::setAscending(bool value)
{
    if (mAscending != value) {
        mAscending = value;
        updateSorting();
        Q_EMIT ascendingChanged();
    }
}

QObject *ConversationProxyModel::conversationModel() const
{
    return sourceModel();
}

void ConversationProxyModel::setConversationModel(QObject *value)
{
    // disconnect the previous model
    ConversationAggregatorModel *oldModel = qobject_cast<ConversationAggregatorModel*>(sourceModel());
    if (oldModel) {
        disconnect(oldModel);
    }

    // and handle the new one
    QAbstractItemModel *model = qobject_cast<QAbstractItemModel*>(value);

    if (model) {
        setSourceModel(model);

        ConversationAggregatorModel *conversationModel = qobject_cast<ConversationAggregatorModel*>(value);
        connect(conversationModel,
                SIGNAL(rowsInserted(QModelIndex,int,int)),
                SLOT(processGrouping()));
        connect(conversationModel,
                SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
                SLOT(processGrouping()));
        connect(conversationModel,
                SIGNAL(rowsRemoved(QModelIndex,int,int)),
                SLOT(processGrouping()));
        connect(conversationModel,
                SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                SLOT(processGrouping()));
        connect(conversationModel,
                SIGNAL(modelReset()),
                SLOT(processGrouping()));
        Q_EMIT conversationModelChanged();
    }

    QHash<int, QByteArray> roles = roleNames();
    roles[EventsRole] = "events";
    setRoleNames(roles);

    processGrouping();
}

QString ConversationProxyModel::searchString() const
{
    return mSearchString;
}

void ConversationProxyModel::setSearchString(QString value)
{
    if (value != mSearchString) {
        mSearchString = value;
        invalidateFilter();
        Q_EMIT dataChanged(index(0,0), index(rowCount()-1, 0));
        Q_EMIT searchStringChanged();
    }
}

bool ConversationProxyModel::grouped() const
{
    return mGrouped;
}

void ConversationProxyModel::setGrouped(bool value)
{
    if (value != mGrouped) {
        mGrouped = value;
        mGroupedEntries.clear();
        processGrouping();
        Q_EMIT groupedChanged();
    }
}

bool ConversationProxyModel::showLatestFromGroup() const
{
    return mShowLatestFromGroup;
}

void ConversationProxyModel::setShowLatestFromGroup(bool value)
{
    if (value != mShowLatestFromGroup) {
        mShowLatestFromGroup = value;
        mGroupedEntries.clear();
        processGrouping();
        Q_EMIT showLatestFromGroupChanged();
    }
}

void ConversationProxyModel::updateSorting()
{
    sort(0, mAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
}

QVariant ConversationProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QModelIndex sourceIndex = mapToSource(index);
    ConversationGroup group = groupForSourceIndex(sourceIndex);

    // fill the result using the standard QSortFilterProxyModel data function
    // and overwrite it if necessary
    QVariant result = QSortFilterProxyModel::data(index, role);
    QVariantMap eventMap;

    switch (role) {
    case EventsRole:
        if (!mGrouped) {
            break;
        }

        // convert the event count into QVariantMap
        Q_FOREACH(const QString & key, group.eventCount.keys()) {
            eventMap[key] = group.eventCount[key];
        }

        result = eventMap;
        break;
    case ConversationFeedModel::ItemType:
        if (mGrouped && mSearchString.isEmpty() && !mShowLatestFromGroup) {
            result = "group";
        }
        break;
    case ConversationFeedModel::NewItem:
        if (mGrouped) {
            result = group.newItem;
        }
        break;
    }

    return result;
}

bool ConversationProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    if (!sourceIndex.isValid()) {
        return false;
    }

    /* So this is how filtering is done:
     * - If there is a search term, grouped is used to return just one result
     *   for contact alias and phone number matching, but individual messages are also returned.
     *   If phoneNumber is set, it is used as a filtering criteria,
     *   so if an entry doesn't match a non-empty search string, we return false.
     *
     * - If onlyLatest is true, phoneNumber has no effect and only the latest
     *   message of each conversation is returned.
     *
     * - If phoneNumber is set, it is used to filter which messages will be
     *   displayed.
     */

    const ConversationAggregatorModel *model = qobject_cast<const ConversationAggregatorModel*>(sourceModel());
    // Start by verifying the search string
    if (!mSearchString.isEmpty()) {
        return model->matchesSearch(mSearchString, sourceIndex);
    }

    ConversationFeedItem *item = qobject_cast<ConversationFeedItem*>(sourceIndex.data(ConversationFeedModel::FeedItem).value<QObject*>());
    if (mGrouped) {
        ConversationGroup group = groupForSourceIndex(sourceIndex);
        return (group.displayedRow == sourceRow);
    }

    if (!mFilterProperty.isEmpty() && !mFilterValue.isEmpty()) {
        QString propertyValue = item->property(mFilterProperty.toLatin1().data()).toString();
        return (propertyValue == mFilterValue);
    }

    return true;
}

void ConversationProxyModel::processGrouping()
{
    if (!sourceModel()) {
        return;
    }

    ConversationAggregatorModel *model = qobject_cast<ConversationAggregatorModel*>(sourceModel());

    mGroupedEntries.clear();

    int count = model->rowCount();
    for (int row = 0; row < count; ++row) {
        QModelIndex sourceIndex = model->index(row, 0, QModelIndex());
        ConversationFeedItem *item = qobject_cast<ConversationFeedItem*>(sourceIndex.data(ConversationFeedModel::FeedItem).value<QObject*>());
        QString groupingProperty = sourceIndex.data(ConversationFeedModel::GroupingProperty).toString();
        QString propertyValue = item->property(groupingProperty.toLatin1().data()).toString();
        ConversationGroup &group = mGroupedEntries[groupingProperty][propertyValue];
        group.eventCount[model->itemType(sourceIndex)]++;
        group.newItem  = group.newItem || item->newItem();

        if (item->timestamp() > group.latestTime || group.displayedRow < 0) {
            group.latestTime = item->timestamp();
            group.displayedRow = row;
        }
    }

    // notify changes for all items to make sure everything gets updated correctly
    Q_FOREACH(const QString &prop, mGroupedEntries.keys()) {
        Q_FOREACH(const ConversationGroup &group, mGroupedEntries[prop].values()) {
            QModelIndex index = mapFromSource(model->index(group.displayedRow, 0, QModelIndex()));
            Q_EMIT dataChanged(index, index);
        }
    }

    invalidateFilter();
}

ConversationGroup ConversationProxyModel::groupForSourceIndex(const QModelIndex &sourceIndex) const
{
    ConversationFeedItem *item = qobject_cast<ConversationFeedItem*>(sourceIndex.data(ConversationFeedModel::FeedItem).value<QObject*>());
    QString groupingProperty = sourceIndex.data(ConversationFeedModel::GroupingProperty).toString();
    QString propertyValue = item->property(groupingProperty.toLatin1().data()).toString();
    ConversationGroup group = mGroupedEntries[groupingProperty][propertyValue];
    return group;
}
