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

#ifndef CONVERSATIONAGGREGATORMODEL_H
#define CONVERSATIONAGGREGATORMODEL_H

#include <QAbstractListModel>

class ConversationFeedModel;

class ConversationAggregatorModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ConversationAggregatorModel(QObject *parent = 0);

    void addFeedModel(ConversationFeedModel *model);
    void removeFeedModel(ConversationFeedModel *model);

    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent) const;

    QModelIndex mapFromSource(const QModelIndex &index) const;
    QModelIndex mapToSource(const QModelIndex &index) const;

    QString itemType(const QModelIndex &index) const;
    bool matchesSearch(const QString &searchTerm, const QModelIndex &index) const;
    QString groupingKeyForIndex(const QModelIndex &index) const;

private Q_SLOTS:
    void updateOffsets();
    void onRowsInserted(const QModelIndex &parent, int start, int end);
    void onRowsRemoved(const QModelIndex &parent, int start, int end);
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void onModelReset();

private:
    QList<ConversationFeedModel*> mFeedModels;
    QMap<const ConversationFeedModel*,int> mModelOffsets;
    int mRowCount;
};

#endif // CONVERSATIONAGGREGATORMODEL_H
