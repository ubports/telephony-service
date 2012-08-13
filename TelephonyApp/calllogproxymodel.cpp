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

#include "calllogproxymodel.h"
#include "calllogmodel.h"
#include <QDebug>

CallLogProxyModel::CallLogProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent), mOnlyMissedCalls(false), mFilterByContact(false)
{
    setDynamicSortFilter(true);
    setSortRole(AbstractLoggerModel::Timestamp);
    sort(0, Qt::DescendingOrder);
}

bool CallLogProxyModel::onlyMissedCalls() const
{
    return mOnlyMissedCalls;
}

QString CallLogProxyModel::contactId() const
{
    return mContactId;
}

bool CallLogProxyModel::filterByContact() const
{
    return mFilterByContact;
}

QObject *CallLogProxyModel::logModel() const
{
    return sourceModel();
}

void CallLogProxyModel::setOnlyMissedCalls(bool value)
{
    if (value != mOnlyMissedCalls) {
        mOnlyMissedCalls = value;
        Q_EMIT onlyMissedCallsChanged();
        invalidateFilter();
    }
}

void CallLogProxyModel::setContactId(QString id)
{
    if (id != mContactId) {
        mContactId = id;
        Q_EMIT contactIdChanged();
        invalidateFilter();
    }
}

void CallLogProxyModel::setFilterByContact(bool value)
{
    if (value != mFilterByContact) {
        mFilterByContact = value;
        Q_EMIT filterByContactChanged();
        invalidateFilter();
    }
}

void CallLogProxyModel::setLogModel(QObject *obj)
{
    QAbstractItemModel *model = qobject_cast<QAbstractItemModel*>(obj);

    if (model) {
        setSourceModel(model);
        Q_EMIT logModelChanged();
    }
}

bool CallLogProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    // Filter the contact id, if set
    if (mFilterByContact) {
        if (mContactId.isEmpty() || sourceIndex.data(CallLogModel::ContactId).toString() != mContactId) {
            return false;
        }
    }

    // filter the missed calls
    if (mOnlyMissedCalls) {
        if (!sourceIndex.data(CallLogModel::Missed).toBool()) {
            return false;
        }
    }

    return true;
}
