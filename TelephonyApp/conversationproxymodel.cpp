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
    QSortFilterProxyModel(parent), mAscending(true), mGrouped(false),
    mShowLatestFromGroup(false), mRequestedDataChanged(false)
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
                SLOT(onRowsInserted(QModelIndex,int,int)));
        connect(conversationModel,
                SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
                SLOT(onRowsMoved(QModelIndex,int,int,QModelIndex,int)));
        connect(conversationModel,
                SIGNAL(rowsRemoved(QModelIndex,int,int)),
                SLOT(onRowsRemoved(QModelIndex,int,int)));
        connect(conversationModel,
                SIGNAL(modelReset()),
                SLOT(processGrouping()));
        connect(conversationModel,
                SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                SLOT(onDataChanged(QModelIndex,QModelIndex)));
        Q_EMIT conversationModelChanged();
    }

    QHash<int, QByteArray> roles = roleNames();
    roles[EventsRole] = "events";
    roles[TimeSlot] = "timeSlot";
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
    ConversationFeedItem *item = qobject_cast<ConversationFeedItem*>(sourceIndex.data(ConversationFeedModel::FeedItem).value<QObject*>());

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

    case TimeSlot:
        result = item->property("timeSlot");
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
        if (mFilterProperty == "phoneNumber") {
            return ContactModel::comparePhoneNumbers(mFilterValue, propertyValue);
        }
        return (propertyValue == mFilterValue);
    }

    return true;
}

ConversationGroup & ConversationProxyModel::groupForEntry(const QString &groupingProperty, const QString &propertyValue) {
    QString finalValue;
    QMap<QString, ConversationGroup> &groups = mGroupedEntries[groupingProperty];
    if (groupingProperty == "phoneNumber") {
        Q_FOREACH(const QString &key, groups.keys()) {
            if (ContactModel::comparePhoneNumbers(key, propertyValue)) {
                mPhoneMatch[propertyValue] = key;
                finalValue = key;
                break;
            }
        }
    }
    if (finalValue.isEmpty()) {
        finalValue = propertyValue;
    }
    ConversationGroup &group = groups[finalValue];
    return group;
}

void ConversationProxyModel::removeGroup(const QString &groupingProperty, const QString &propertyValue)
{
    if (groupingProperty == "phoneNumber") {
        QMap<QString, ConversationGroup> &groups = mGroupedEntries[groupingProperty];
        Q_FOREACH(const QString &key, groups.keys()) {
            if (ContactModel::comparePhoneNumbers(key, propertyValue)) {
                groups.remove(key);
                break;
            }
        }
    } else {
         mGroupedEntries[groupingProperty].remove(propertyValue);
    }
}

void ConversationProxyModel::processGrouping()
{
    if (!sourceModel()) {
        return;
    }

    ConversationAggregatorModel *model = qobject_cast<ConversationAggregatorModel*>(sourceModel());

    mGroupedEntries.clear();
    mPhoneMatch.clear();
    int count = model->rowCount();
    for (int row = 0; row < count; ++row) {
        processRowGrouping(row, false);
    }

    // create the time slots after the filtering has been updated
    processTimeSlots(false);

    // only after processing all the rows we emit the dataChanged signal
    for (int row = 0; row < count; ++row) {
        emitDataChanged(model->index(row));
    }
}

void ConversationProxyModel::processRowGrouping(int sourceRow, bool notify)
{
    ConversationAggregatorModel *model = qobject_cast<ConversationAggregatorModel*>(sourceModel());
    QModelIndex sourceIndex = model->index(sourceRow, 0, QModelIndex());
    ConversationFeedItem *item = qobject_cast<ConversationFeedItem*>(sourceIndex.data(ConversationFeedModel::FeedItem).value<QObject*>());
    QString groupingProperty = sourceIndex.data(ConversationFeedModel::GroupingProperty).toString();
    QString propertyValue = item->property(groupingProperty.toLatin1().data()).toString();
    ConversationGroup &group = groupForEntry(groupingProperty, propertyValue);

    // save the previous grouping property and value in case the item data changes
    item->setProperty("groupingProperty", groupingProperty);
    item->setProperty("propertyValue", propertyValue);

    group.eventCount[model->itemType(sourceIndex)]++;
    group.newItem  = group.newItem || item->newItem();
    if (!group.rows.contains(sourceRow)) {
        group.rows.append(sourceRow);
    }

    if (item->timestamp() > group.latestTime || group.displayedRow < 0) {
        int oldDisplayedRow = group.displayedRow;
        group.latestTime = item->timestamp();
        group.displayedRow = sourceRow;


        // if the displayed row changed, report that it changed
        if (oldDisplayedRow >= 0 && oldDisplayedRow != sourceRow) {
            QModelIndex index = model->index(oldDisplayedRow);
            if (notify) {
                emitDataChanged(index);
            }
        }
    }

    if (notify) {
        emitDataChanged(sourceIndex);
    }
}

void ConversationProxyModel::removeRowFromGroup(int sourceRow, QString groupingProperty, QString propertyValue)
{
    ConversationAggregatorModel *model = qobject_cast<ConversationAggregatorModel*>(sourceModel());
    QModelIndex sourceIndex = model->index(sourceRow, 0, QModelIndex());
    ConversationFeedItem *item = qobject_cast<ConversationFeedItem*>(sourceIndex.data(ConversationFeedModel::FeedItem).value<QObject*>());
    if (groupingProperty.isEmpty()) {
        groupingProperty = sourceIndex.data(ConversationFeedModel::GroupingProperty).toString();
        propertyValue = item->property(groupingProperty.toLatin1().data()).toString();
    }
    ConversationGroup &group = groupForEntry(groupingProperty, propertyValue);

    group.rows.removeAll(sourceRow);
    if (group.displayedRow == sourceRow) {
        // find the newest event
        int newRow = -1;
        QDateTime latestTimestamp;
        Q_FOREACH(int i, group.rows) {
            QDateTime timestamp = model->index(i).data(ConversationFeedModel::Timestamp).toDateTime();
            if (timestamp > latestTimestamp) {
                newRow = i;
                latestTimestamp = timestamp;
            }
        }

        if (newRow >= 0) {
            group.displayedRow = newRow;
            group.latestTime = latestTimestamp;

            QModelIndex index = model->index(newRow);
            emitDataChanged(index);
        } else if (group.rows.isEmpty()) {
            removeGroup(groupingProperty, propertyValue);
        }
    }
}

void ConversationProxyModel::processTimeSlots(int start, bool notify)
{
    int count = rowCount();
    if (mFilterProperty.isEmpty() || count == 0) {
        return;
    }

    QDateTime currentSlot = index(0, 0).data(ConversationFeedModel::Timestamp).toDateTime();
    for (int i = 0; i < count; ++i) {
        QModelIndex idx = index(i, 0);
        ConversationFeedItem *item = qobject_cast<ConversationFeedItem*>(idx.data(ConversationFeedModel::FeedItem).value<QObject*>());
        QDateTime itemTimestamp = item->timestamp();
        int minutes = itemTimestamp.secsTo(currentSlot) / 60;

        if (minutes > 15) {
            currentSlot = itemTimestamp;
        }

        // check the current timeslot
        QDateTime itemSlot = item->property("timeSlot").toDateTime();
        if (itemSlot != currentSlot) {
            item->setProperty("timeSlot", currentSlot);
            if (notify) {
                Q_EMIT dataChanged(idx, idx);
            }
        }
    }
}

void ConversationProxyModel::emitDataChanged(const QModelIndex &sourceIndex)
{
    mRequestedDataChanged = true;
    Q_EMIT sourceModel()->dataChanged(sourceIndex, sourceIndex);
    mRequestedDataChanged = false;
}

void ConversationProxyModel::onRowsInserted(const QModelIndex &parent, int start, int end)
{
    // we don't support tree models yet
    if (parent.isValid()) {
        return;
    }

    // update the group for the added indexes
    for (int row = start; row <= end; ++row) {
        processRowGrouping(row);
    }

    processTimeSlots();
}

void ConversationProxyModel::onRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &newParent, int newRow)
{
    // we don't support tree models yet
    if (parent.isValid() || newParent.isValid()) {
        return;
    }

    ConversationAggregatorModel *model = qobject_cast<ConversationAggregatorModel*>(sourceModel());
    int offset = newRow = start;
    for (int row = start; row <= end; ++row) {
        QModelIndex sourceIndex = model->index(row, 0, QModelIndex());
        ConversationFeedItem *item = qobject_cast<ConversationFeedItem*>(sourceIndex.data(ConversationFeedModel::FeedItem).value<QObject*>());
        QString groupingProperty = sourceIndex.data(ConversationFeedModel::GroupingProperty).toString();
        QString propertyValue = item->property(groupingProperty.toLatin1().data()).toString();
        ConversationGroup &group = groupForEntry(groupingProperty, propertyValue);

        // if the currently displayed row is moved, we need to update that.
        if (group.displayedRow == row) {
            group.displayedRow = row + offset;

            QModelIndex index = model->index(group.displayedRow);
            emitDataChanged(index);

            index = mapFromSource(model->index(row));
            emitDataChanged(index);
        }

        group.rows.removeAll(row);
        group.rows.append(row + offset);
    }

    processTimeSlots();
}

void ConversationProxyModel::onRowsRemoved(const QModelIndex &parent, int start, int end)
{
    // we don't support tree models yet
    if (parent.isValid()) {
        return;
    }

    for (int row = start; row <= end; ++row) {
        removeRowFromGroup(row);
    }

    processTimeSlots();
}

void ConversationProxyModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (mRequestedDataChanged) {
        return;
    }

    // we don't support tree models yet
    if (topLeft.parent().isValid() || bottomRight.parent().isValid()) {
        return;
    }

    int start = topLeft.row();
    int end = bottomRight.row();
    ConversationAggregatorModel *model = qobject_cast<ConversationAggregatorModel*>(sourceModel());

    for (int row = start; row <= end; ++row) {
        QModelIndex sourceIndex = model->index(row, 0, QModelIndex());
        ConversationFeedItem *item = qobject_cast<ConversationFeedItem*>(sourceIndex.data(ConversationFeedModel::FeedItem).value<QObject*>());
        QString groupingProperty = sourceIndex.data(ConversationFeedModel::GroupingProperty).toString();
        QString propertyValue = item->property(groupingProperty.toLatin1().data()).toString();
        QString oldGroupingProperty = item->property("groupingProperty").toString();
        QString oldPropertyValue = item->property("propertyValue").toString();

        // if the grouping property changed, we need to update the item accordingly
        if (oldGroupingProperty != groupingProperty || oldPropertyValue != propertyValue) {
            removeRowFromGroup(row, oldGroupingProperty, oldPropertyValue);
            processRowGrouping(row);
        }
    }
    processTimeSlots();
}


ConversationGroup ConversationProxyModel::groupForSourceIndex(const QModelIndex &sourceIndex) const
{
    ConversationFeedItem *item = qobject_cast<ConversationFeedItem*>(sourceIndex.data(ConversationFeedModel::FeedItem).value<QObject*>());
    QString groupingProperty = sourceIndex.data(ConversationFeedModel::GroupingProperty).toString();
    QString propertyValue = item->property(groupingProperty.toLatin1().data()).toString();
    
    if(groupingProperty == "phoneNumber" && !mPhoneMatch[propertyValue].isEmpty()) {
        return mGroupedEntries[groupingProperty][mPhoneMatch.value(propertyValue)];
    } else {
        return mGroupedEntries[groupingProperty][propertyValue];
    }
}
