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
public:
    explicit MessagesProxyModel(QObject *parent = 0);
    
    bool ascending() const;
    void setAscending(bool value);

    QObject *messagesModel() const;
    void setMessagesModel(QObject *value);

    void updateSorting();

signals:
    void ascendingChanged();
    void messagesModelChanged();

private:
    bool mAscending;
};

#endif // MESSAGESPROXYMODEL_H
