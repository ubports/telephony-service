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

#ifndef CALLLOGMODEL_H
#define CALLLOGMODEL_H

#include <QAbstractListModel>
#include <TelepathyLoggerQt4/PendingOperation>
#include <QDateTime>
#include <QList>
#include <QUrl>
#include <QContact>
#include <QContactId>

using namespace QtMobility;

class CallEntry {
public:
    QContactLocalId localId;
    QString contactId;
    QString contactAlias;
    QUrl avatar;
    QString phoneNumber;
    QString phoneType;
    QDateTime timestamp;
    bool missed;
    bool incoming;
};

namespace QtMobility {
    class QContactManager;
}

class CallLogModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum CallLogRoles {
        ContactId = Qt::UserRole,
        ContactAlias,
        Avatar,
        PhoneNumber,
        PhoneType,
        Timestamp,
        Missed,
        Incoming
    };

    explicit CallLogModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    
protected:
    void fetchCallLog();
    void fillContactInfo(CallEntry &entry, const QContact &contact);
    void clearContactInfo(CallEntry &entry);

protected slots:
    void onPendingEntitiesFinished(Tpl::PendingOperation *op);
    void onPendingDatesFinished(Tpl::PendingOperation *op);
    void onPendingEventsFinished(Tpl::PendingOperation *op);

    // QContactManager related slots
    void onContactsAdded(const QList<QContactLocalId> &contactIds);
    void onContactsChanged(const QList<QContactLocalId> &contactIds);
    void onContactsRemoved(const QList<QContactLocalId> &contactIds);

private:
    QList<CallEntry> mCallEntries;
    QContactManager *mContactManager;

};

#endif // CALLLOGMODEL_H
