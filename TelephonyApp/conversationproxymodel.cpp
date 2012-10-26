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
    QAbstractItemModel *model = qobject_cast<QAbstractItemModel*>(value);

    if (model) {
        setSourceModel(model);

        ConversationAggregatorModel *conversationModel = qobject_cast<ConversationAggregatorModel*>(value);
        if (conversationModel) {
            connect(conversationModel, SIGNAL(resetView()), SLOT(onResetView()));
        }

        Q_EMIT conversationModelChanged();
    }
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
    // TODO: implement the event count roles
    return QSortFilterProxyModel::data(index, role);
}

bool ConversationProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    if (!sourceIndex.isValid()) {
        return false;
    }

    /* So this is how filtering is done:
     * - If there is a search term, onlyLatest is used to return just one result
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
    QString groupingProperty = "contactId";
    // TODO: fix the code for grouping contacts
    /*if (mGrouped) {
        if (item->contactId().isEmpty()) {
            groupingProperty = model->groupingKeyForIndex(sourceIndex);
        }

        ConversationGroup group;
        bool displayed = true;
        char *property = groupingProperty.toLatin1().data();
        if (mGroupedEntries[groupingProperty].contains(item->property(property).toString())) {
            group = mGroupedEntries[groupingProperty][item->property(property).toString()];
            if (item->timestamp() > group.latestTime) {
                group.latestTime = item->timestamp();
            }
            displayed = false;
        } else {
            group.displayedRow = sourceIndex.row();
            group.latestTime = item->timestamp();
        }
        group.eventCount[model->itemType(sourceIndex)]++;


        mGroupedEntries[groupingProperty][item->property(property).toString()] = group;
        QModelIndex index = mapFromSource(model->index(group.displayedRow));
        Q_EMIT dataChanged(index, index);

        return displayed;
    }*/

    if (!mFilterProperty.isEmpty() && !mFilterValue.isEmpty()) {
        char *property = mFilterProperty.toLatin1().data();
        QString itemProperty = item->property(property).toString();
        return (itemProperty == mFilterValue);
    }

    return true;
}
