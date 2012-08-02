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
#include "chatmanager.h"
#include "contactmodel.h"

MessagesProxyModel::MessagesProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent), mAscending(true), mOnlyLatest(false)
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
    // when the filter changes we also have to reset the model as the
    // ListView element seems to not behave correctly when rows
    // are changed. The same is done for setThreadId().
    reset();
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
    reset();
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

        QHash<int, QByteArray> roles = roleNames();
        roles[UnreadCount] = "unreadCount";
        setRoleNames(roles);

        emit messagesModelChanged();
        MessageLogModel *loggerModel = qobject_cast<MessageLogModel*>(value);
        if (loggerModel) {
            connect(loggerModel, SIGNAL(resetView()), SLOT(onResetView()));
        }
    }
}

void MessagesProxyModel::onResetView()
{
    reset();
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
        emit dataChanged(index(0,0), index(rowCount()-1, 0));
        emit searchStringChanged();
    }
}

bool MessagesProxyModel::onlyLatest() const
{
    return mOnlyLatest;
}

void MessagesProxyModel::setOnlyLatest(bool value)
{
    if (value != mOnlyLatest) {
        mOnlyLatest = value;
        invalidateFilter();
        emit onlyLatestChanged();
    }
}

void MessagesProxyModel::updateSorting()
{
    sort(0, mAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
}

QVariant MessagesProxyModel::data(const QModelIndex &index, int role) const
{
    if (role == UnreadCount) {
        // if we are searching or if we are showing not just the latest message,
        // we don't show messages as unread
        if (!mOnlyLatest || !mSearchString.isEmpty()) {
            return 0;
        }

        QString number = QSortFilterProxyModel::data(index, AbstractLoggerModel::PhoneNumber).toString();
        return ChatManager::instance()->unreadMessages(number);
    }

    return QSortFilterProxyModel::data(index, role);
}

bool MessagesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    if (!sourceIndex.isValid()) {
        return false;
    }

    /* So this is how filtering is done:
     * - If there is a search term, onlyLatest is used to return just one result
     *   for contact alias and phone number matching, but individual messages are also returned.
     *   If threadId and phoneNumber are set, they are used as a filtering criteria,
     *   so if an entry doesn't match a non-empty search string, we return false.
     *
     * - If onlyLatest is true, phoneNumber and threadId have no effect and only the latest
     *   message of each thread is returned.
     *
     * - If threadId or phoneNumber are set, they are used to filter which messages will be
     *   displayed.
     */

    bool foundMatch = false;

    // Start by verifying the search string
    if (!mSearchString.isEmpty()) {
        // Test the contact alias
        QString value = sourceIndex.data(AbstractLoggerModel::ContactAlias).toString();
        if (value.indexOf(mSearchString, 0, Qt::CaseInsensitive) >= 0) {
            // if onlyLatest option is set, we just return one contact alias match
            foundMatch = mOnlyLatest ? sourceIndex.data(MessageLogModel::IsLatest).toBool() : true;
        }

        // Test the phone number
        value = sourceIndex.data(AbstractLoggerModel::PhoneNumber).toString();
        if (ContactModel::instance()->comparePhoneNumbers(value, mSearchString)) {
            // if onlyLatest option is set, we just return one contact alias match
            foundMatch = mOnlyLatest ? sourceIndex.data(MessageLogModel::IsLatest).toBool() : true;
        }

        // Test the message text. Even if onlyLatest is set, we return all text entries that match
        value = sourceIndex.data(MessageLogModel::Message).toString();
        if (value.indexOf(mSearchString, 0, Qt::CaseInsensitive) >= 0) {
            foundMatch = true;
        }
    } else if (mOnlyLatest) {
        // search string is empty, so check for onlyLatest
        // at this point we can just return true or false, no need
        // to further check for threadId and phone number
        return sourceIndex.data(MessageLogModel::IsLatest).toBool();
    } else {
        // no specific criteria, consider all items matching for further evaluation
        foundMatch = true;
    }

    // if the items don't match the criteria above, do not display them
    if (!foundMatch) {
        return false;
    }

    // after checking for all the conditions above, if the item match either the
    // threadId or the phoneNumber, we can display it.
    if (!mThreadId.isEmpty()) {
        QString value = sourceIndex.data(MessageLogModel::ThreadId).toString();
        if (value == mThreadId) {
            return true;
        }
    }
    
    if (!mPhoneNumber.isEmpty()) {
        QString phoneNumber = sourceIndex.data(AbstractLoggerModel::PhoneNumber).toString();
        return ContactModel::instance()->comparePhoneNumbers(mPhoneNumber, phoneNumber);
    }

    // if both mThreadId and mPhoneNumber are empty at this point all items can be displayed.
    return mPhoneNumber.isEmpty() && mThreadId.isEmpty();
}

void MessagesProxyModel::onUnreadMessagesChanged(const QString &number)
{
    // if we are searching, or showing all messages, do nothing
    if (!mOnlyLatest || !mSearchString.isEmpty()) {
        return;
    }

    int count = rowCount();
    for (int i =0; i < count; ++i) {
        QModelIndex idx = index(i, 0);
        QString phoneNumber = idx.data(AbstractLoggerModel::PhoneNumber).toString();
        if (ContactModel::instance()->comparePhoneNumbers(phoneNumber, number)) {
            emit dataChanged(idx, idx);
        }
    }
}
