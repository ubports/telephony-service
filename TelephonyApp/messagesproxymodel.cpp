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

#include "messagesproxymodel.h"
#include "abstractloggermodel.h"

MessagesProxyModel::MessagesProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent), mAscending(true)
{
    setSortRole(AbstractLoggerModel::Timestamp);
    updateSorting();
}

bool MessagesProxyModel::ascending() const
{
    return mAscending;
}

void MessagesProxyModel::setAscending(bool value)
{
    if (mAscending != value) {
        mAscending = value;
        updateSorting();
        emit ascendingChanged();
    }
}

QObject *MessagesProxyModel::messagesModel() const
{
    return sourceModel();
}

void MessagesProxyModel::setMessagesModel(QObject *value)
{
    QAbstractItemModel *model = qobject_cast<QAbstractItemModel*>(value);

    if (model) {
        setSourceModel(model);
        emit messagesModelChanged();
    }
}

void MessagesProxyModel::updateSorting()
{
    sort(0, mAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
}
