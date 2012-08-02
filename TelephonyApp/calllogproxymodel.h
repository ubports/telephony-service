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

#ifndef CALLLOGPROXYMODEL_H
#define CALLLOGPROXYMODEL_H

#include <QSortFilterProxyModel>

class CallLogProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool onlyMissedCalls
               READ onlyMissedCalls
               WRITE setOnlyMissedCalls
               NOTIFY onlyMissedCallsChanged)
    Q_PROPERTY(QString contactId
               READ contactId
               WRITE setContactId
               NOTIFY contactIdChanged)
    Q_PROPERTY(bool filterByContact
               READ filterByContact
               WRITE setFilterByContact
               NOTIFY filterByContactChanged)
    Q_PROPERTY(QObject *logModel
               READ logModel
               WRITE setLogModel
               NOTIFY logModelChanged)
public:
    explicit CallLogProxyModel(QObject *parent = 0);

    bool onlyMissedCalls() const;
    QString contactId() const;
    bool filterByContact() const;
    QObject *logModel() const;

public slots:
    void setOnlyMissedCalls(bool value);
    void setContactId(QString id);
    void setFilterByContact(bool value);
    void setLogModel(QObject *model);

signals:
    void onlyMissedCallsChanged();
    void contactIdChanged();
    void filterByContactChanged();
    void logModelChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    bool mOnlyMissedCalls;
    QString mContactId;
    bool mFilterByContact;
};

#endif // CALLLOGPROXYMODEL_H
