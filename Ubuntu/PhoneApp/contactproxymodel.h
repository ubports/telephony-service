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

#ifndef CONTACTPROXYMODEL_H
#define CONTACTPROXYMODEL_H

#include <QSortFilterProxyModel>

class ContactProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QObject *model
               READ model
               WRITE setModel
               NOTIFY modelChanged)
    Q_PROPERTY(QString filterText
               READ filterText
               WRITE setFilterText
               NOTIFY filterTextChanged)
    Q_PROPERTY(bool favoriteOnly
               READ favoriteOnly
               WRITE setFavoriteOnly
               NOTIFY favoriteOnlyChanged)
    Q_PROPERTY(int count
               READ rowCount
               NOTIFY rowCountChanged)

public:
    explicit ContactProxyModel(QObject *parent = 0);
    
    QObject *model() const;
    void setModel(QObject *value);

    QString filterText() const;
    void setFilterText(const QString &value);
    void setFavoriteOnly(bool enable);
    bool favoriteOnly();

protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

Q_SIGNALS:
    void modelChanged();
    void filterTextChanged();
    void favoriteOnlyChanged();
    void rowCountChanged();

private:
    QString mFilterText;
    bool mFavoriteOnly;
    
};

#endif // CONTACTPROXYMODEL_H
