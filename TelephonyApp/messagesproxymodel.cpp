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
#include "messagelogmodel.h"
#include "contactmodel.h"

MessagesProxyModel::MessagesProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent), mAscending(true)
{
    setSortRole(AbstractLoggerModel::Timestamp);
    setDynamicSortFilter(true);
    updateSorting();
}

QString MessagesProxyModel::phoneNumber() const
{
    return mPhoneNumber;
}

void MessagesProxyModel::setPhoneNumber(const QString &value)
{
    mPhoneNumber = value;
    invalidateFilter();
    emit phoneNumberChanged();
}

QString MessagesProxyModel::threadId() const
{
    return mThreadId;
}

void MessagesProxyModel::setThreadId(const QString &value)
{
    mThreadId = value;
    invalidateFilter();
    emit threadIdChanged();
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

QString MessagesProxyModel::searchString() const
{
    return mSearchString;
}

void MessagesProxyModel::setSearchString(QString value)
{
    if (value != mSearchString) {
        mSearchString = value;
        invalidateFilter();
        emit searchStringChanged();
    }
}

void MessagesProxyModel::updateSorting()
{
    sort(0, mAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
}

bool MessagesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    if (!sourceIndex.isValid()) {
        return false;
    }


    if (!mThreadId.isEmpty()) {
        QString value = sourceIndex.data(MessageLogModel::ThreadId).toString();
        if (value == mThreadId) {
            return true;
        } else {
            QString phoneNumber = sourceIndex.data(AbstractLoggerModel::PhoneNumber).toString();
            return ContactModel::instance()->comparePhoneNumbers(mPhoneNumber, phoneNumber);
        }
    }
    
    if (!mPhoneNumber.isEmpty()) {
        QString phone = sourceIndex.data(AbstractLoggerModel::PhoneNumber).toString();
        return ContactModel::instance()->comparePhoneNumbers(mPhoneNumber, phone);
    }

    if (mSearchString.isEmpty()) {
        return true;
    }

    // test the contact alias
    QString value = sourceIndex.data(AbstractLoggerModel::ContactAlias).toString();
    if (value.indexOf(mSearchString, 0, Qt::CaseInsensitive) >= 0) {
        return true;
    }

    // test the phone number
    value = sourceIndex.data(AbstractLoggerModel::PhoneNumber).toString();
    // FIXME: use a more reliable way to compare the phone number
    if (value.indexOf(mSearchString, 0, Qt::CaseInsensitive) >= 0) {
        return true;
    }

    // test the message text
    value = sourceIndex.data(MessageLogModel::Message).toString();
    if (value.indexOf(mSearchString, 0, Qt::CaseInsensitive) >= 0) {
        return true;
    }

    return false;
}
