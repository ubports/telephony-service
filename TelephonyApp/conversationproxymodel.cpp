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
    QSortFilterProxyModel(parent), mAscending(true), mGrouped(false)
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
        if (conversationModel) {
            connect(conversationModel, SIGNAL(resetView()), SLOT(onResetView()));
        }

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

void ConversationProxyModel::onResetView()
{
    reset();
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
        invalidateFilter();
        Q_EMIT groupedChanged();
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

    switch (role) {
    case EventsRole: {
        if (!mGrouped) {
            return QVariant();
        }

        ConversationGroup group = groupForSourceIndex(sourceIndex);

        // convert the event count into QVariantMap
        QVariantMap eventMap;
        Q_FOREACH(const QString & key, group.eventCount.keys()) {
            eventMap[key] = group.eventCount[key];
        }

        return eventMap;
    }
    case ConversationFeedModel::Timestamp:
        if (mGrouped) {
            ConversationGroup group = groupForSourceIndex(sourceIndex);
            return group.latestTime;
        }
        return QSortFilterProxyModel::data(index, role);
    case ConversationFeedModel::ItemType:
        if (mGrouped && mSearchString.isEmpty()) {
            return "group";
        }
    default:
        return QSortFilterProxyModel::data(index, role);
    }
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

        if (item->timestamp() > group.latestTime) {
            group.latestTime = item->timestamp();
        }

        if (group.displayedRow < 0) {
            group.displayedRow = row;
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
