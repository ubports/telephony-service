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

#ifndef MESSAGESPROXYMODEL_H
#define MESSAGESPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "messagelogmodel.h"

class MessagesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(bool ascending
               READ ascending
               WRITE setAscending
               NOTIFY ascendingChanged)
    Q_PROPERTY(QObject *messagesModel
               READ messagesModel
               WRITE setMessagesModel
               NOTIFY messagesModelChanged)
    Q_PROPERTY (QString searchString
                READ searchString
                WRITE setSearchString
                NOTIFY searchStringChanged)
    Q_PROPERTY(QString phoneNumber
               READ phoneNumber
               WRITE setPhoneNumber
               NOTIFY phoneNumberChanged)
    Q_PROPERTY(QString threadId
               READ threadId
               WRITE setThreadId
               NOTIFY threadIdChanged)
    Q_PROPERTY(bool onlyLatest
               READ onlyLatest
               WRITE setOnlyLatest
               NOTIFY onlyLatestChanged)

    Q_ENUMS(ModelRoles)

public:
    enum ModelRoles {
        UnreadCount = MessageLogModel::LastMessageRole
    };

    explicit MessagesProxyModel(QObject *parent = 0);

    QString phoneNumber() const;
    void setPhoneNumber(const QString &value);

    QString threadId() const;
    void setThreadId(const QString &value);

    bool ascending() const;
    void setAscending(bool value);

    QObject *messagesModel() const;
    void setMessagesModel(QObject *value);

    QString searchString() const;
    void setSearchString(QString value);

    bool onlyLatest() const;
    void setOnlyLatest(bool value);

    void updateSorting();

    virtual QVariant data(const QModelIndex &index, int role) const;

private Q_SLOTS:
    void onResetView();

signals:
    void ascendingChanged();
    void messagesModelChanged();
    void searchStringChanged();
    void phoneNumberChanged();
    void threadIdChanged();
    void onlyLatestChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

protected Q_SLOTS:
    void onUnreadMessagesChanged(const QString &number);

private:
    bool mAscending;
    QString mSearchString;
    QString mPhoneNumber;
    QString mThreadId;
    bool mOnlyLatest;
};

#endif // MESSAGESPROXYMODEL_H
