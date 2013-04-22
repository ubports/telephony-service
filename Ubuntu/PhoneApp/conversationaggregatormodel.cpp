/*
 * Copyright (C) 2012 Canonical, Ltd.
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

#include "conversationaggregatormodel.h"
#include "conversationfeedmodel.h"
#include <QDebug>

ConversationAggregatorModel::ConversationAggregatorModel(QObject *parent) :
    QAbstractListModel(parent), mRowCount(0)
{
    mRoles = QAbstractListModel::roleNames();
    mRoles[ConversationFeedModel::ContactId] = "contactId";
    mRoles[ConversationFeedModel::ContactAlias] = "contactAlias";
    mRoles[ConversationFeedModel::ContactAvatar] = "contactAvatar";
    mRoles[ConversationFeedModel::Timestamp] = "timestamp";
    mRoles[ConversationFeedModel::Date] = "date";
    mRoles[ConversationFeedModel::Incoming] = "incoming";
    mRoles[ConversationFeedModel::NewItem] = "newItem";
    mRoles[ConversationFeedModel::ItemType] = "itemType";
    mRoles[ConversationFeedModel::FeedItem] = "item";
    mRoles[ConversationFeedModel::GroupingProperty] = "groupingProperty";
}

void ConversationAggregatorModel::addFeedModel(ConversationFeedModel *model)
{
    if (mFeedModels.contains(model)) {
        return;
    }

    int offset = 0;
    if (mFeedModels.count() > 0) {
        ConversationFeedModel *lastModel = mFeedModels.last();
        offset = mModelOffsets[lastModel] + lastModel->rowCount();
    }

    if (model->rowCount() > 0) {
        beginInsertRows(QModelIndex(), offset, offset + model->rowCount() - 1);
        mFeedModels.append(model);
        updateOffsets();
        endInsertRows();
    } else {
        mFeedModels.append(model);
        updateOffsets();
    }

    connect(model,
            SIGNAL(rowsInserted(QModelIndex,int,int)),
            SLOT(onRowsInserted(QModelIndex,int,int)));
    connect(model,
            SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SLOT(onRowsRemoved(QModelIndex,int,int)));
    connect(model,
            SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            SLOT(onDataChanged(QModelIndex,QModelIndex)));
    connect(model,
            SIGNAL(modelReset()),
            SLOT(onModelReset()));
}

void ConversationAggregatorModel::removeFeedModel(ConversationFeedModel *model)
{
    if (!mFeedModels.contains(model)) {
        return;
    }

    int offset = mModelOffsets[model];
    int rowCount = model->rowCount();
    if (rowCount > 0) {
        beginRemoveRows(QModelIndex(), offset, offset + rowCount - 1);
    }
    mModelOffsets.remove(model);
    mFeedModels.removeOne(model);
    disconnect(model);
    updateOffsets();
    if (rowCount > 0) {
        endRemoveRows();
    }
}

QVariant ConversationAggregatorModel::data(const QModelIndex &index, int role) const
{
    return mapToSource(index).data(role);
}

int ConversationAggregatorModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return mRowCount;
}

QModelIndex ConversationAggregatorModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return QModelIndex();
    }

    if (row < 0 || row > rowCount()) {
        return QModelIndex();
    }

    // find the model offset
    Q_FOREACH(ConversationFeedModel *model, mFeedModels) {
        // check if the row is in the range of this model
        if (row < mModelOffsets[model] + model->rowCount()) {
            return createIndex(row, column, (void*) model);
        }
    }

    return QModelIndex();
}

QHash<int, QByteArray> ConversationAggregatorModel::roleNames() const
{
    return mRoles;
}

QModelIndex ConversationAggregatorModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) {
        return sourceIndex;
    }

    ConversationFeedModel *model;
    model = const_cast<ConversationFeedModel*>(qobject_cast<const ConversationFeedModel*>(sourceIndex.model()));
    if (!model || !mFeedModels.contains(model)) {
        return QModelIndex();
    }

    return index(sourceIndex.row() + mModelOffsets[model], 0, QModelIndex());
}

QModelIndex ConversationAggregatorModel::mapToSource(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return index;
    }

    ConversationFeedModel *model = static_cast<ConversationFeedModel*>(index.internalPointer());
    return model->index(index.row() - mModelOffsets[model]);
}

QString ConversationAggregatorModel::itemType(const QModelIndex &index) const
{
    ConversationFeedModel *model = static_cast<ConversationFeedModel*>(index.internalPointer());
    return model->itemType(mapToSource(index));
}

bool ConversationAggregatorModel::matchesSearch(const QString &searchTerm, const QModelIndex &index) const
{
    QModelIndex sourceIndex = mapToSource(index);
    if (!sourceIndex.isValid()) {
        return false;
    }

    ConversationFeedModel *model = static_cast<ConversationFeedModel*>(index.internalPointer());
    return model->matchesSearch(searchTerm, sourceIndex);
}

void ConversationAggregatorModel::updateOffsets()
{
    int offset = 0;
    mRowCount = 0;
    Q_FOREACH(ConversationFeedModel *model, mFeedModels) {
        mModelOffsets[model] = offset;
        offset += model->rowCount();
        mRowCount += model->rowCount();
    }
}

void ConversationAggregatorModel::onRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)

    ConversationFeedModel *model = qobject_cast<ConversationFeedModel*>(sender());
    if (!model) {
        return;
    }

    int offset = mModelOffsets[model];
    beginInsertRows(QModelIndex(), start + offset, end + offset);
    updateOffsets();
    endInsertRows();
}

void ConversationAggregatorModel::onRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)
    ConversationFeedModel *model = qobject_cast<ConversationFeedModel*>(sender());
    if (!model) {
        return;
    }

    int offset = mModelOffsets[model];
    beginRemoveRows(QModelIndex(), start + offset, end + offset);
    updateOffsets();
    endRemoveRows();
}

void ConversationAggregatorModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (!topLeft.isValid() || !bottomRight.isValid()) {
        return;
    }

    Q_EMIT dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight));
}

void ConversationAggregatorModel::onModelReset()
{
    beginResetModel();
    updateOffsets();
    endResetModel();
}
