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

#include "conversationaggregatormodel.h"
#include "conversationfeedmodel.h"

ConversationAggregatorModel::ConversationAggregatorModel(QObject *parent) :
    QAbstractListModel(parent), mRowCount(0)
{
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

    beginInsertRows(QModelIndex(), offset, offset + model->rowCount() - 1);
    mFeedModels.append(model);
    updateOffsets();
    endInsertRows();

    connect(model,
            SIGNAL(rowsInserted(QModelIndex,int,int)),
            SLOT(onRowsInserted(QModelIndex,int,int)));
    connect(model,
            SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SLOT(onRowsRemoved(QModelIndex,int,int)));
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
    beginRemoveRows(QModelIndex(), offset, offset + model->rowCount() - 1);
    mModelOffsets.remove(model);
    mFeedModels.removeOne(model);
    endRemoveRows();
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

QModelIndex ConversationAggregatorModel::mapFromSource(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return index;
    }

    ConversationFeedModel *model = const_cast<ConversationFeedModel*>(qobject_cast<const ConversationFeedModel*>(index.model()));
    if (!model || !mFeedModels.contains(model)) {
        return QModelIndex();
    }

    return createIndex(index.row() + mModelOffsets[model], 0, (void*)model);
}

QModelIndex ConversationAggregatorModel::mapToSource(const QModelIndex &index) const
{
    ConversationFeedModel *model = static_cast<ConversationFeedModel*>(index.internalPointer());
    if (!model) {
        return QModelIndex();
    }

    return model->index(index.row() - mModelOffsets[model]);
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
    ConversationFeedModel *model = qobject_cast<ConversationFeedModel*>(sender());
    int offset = mModelOffsets[model];
    beginInsertRows(QModelIndex(), start + offset, end + offset);
    updateOffsets();
    endInsertRows();
}

void ConversationAggregatorModel::onRowsRemoved(const QModelIndex &parent, int start, int end)
{
    ConversationFeedModel *model = qobject_cast<ConversationFeedModel*>(sender());
    int offset = mModelOffsets[model];
    beginRemoveRows(QModelIndex(), start + offset, end + offset);
    updateOffsets();
    endRemoveRows();
}

void ConversationAggregatorModel::onModelReset()
{
    beginResetModel();
    updateOffsets();
    endResetModel();
}
