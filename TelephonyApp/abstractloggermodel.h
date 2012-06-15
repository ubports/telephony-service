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

#ifndef ABSTRACTLOGGERMODEL_H
#define ABSTRACTLOGGERMODEL_H

#include <QAbstractListModel>
#include <TelepathyLoggerQt4/PendingOperation>
#include <TelepathyLoggerQt4/Types>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QUrl>
#include <QContact>
#include <QContactId>
#include <QContactManager>

using namespace QtMobility;

class LogEntry {
public:
    virtual QVariant data(int role) const;

    // for simplicity keep the members as public
    QContactLocalId localId;
    QString contactId;
    QString contactAlias;
    QUrl avatar;
    QString phoneNumber;
    QString phoneType;
    QDateTime timestamp;
    bool incoming;
};

class AbstractLoggerModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum LogRoles {
        ContactId = Qt::UserRole,
        ContactAlias,
        Avatar,
        PhoneNumber,
        PhoneType,
        Timestamp,
        Incoming,
        LastLogRole
    };

    explicit AbstractLoggerModel(QtMobility::QContactManager *manager, QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    
protected:
    void fetchLog(Tpl::EventTypeMask type = Tpl::EventTypeMaskAny);
    void requestDatesForEntities(const Tpl::EntityPtrList &entities);
    void requestEventsForDates(const Tpl::EntityPtr &entity, const Tpl::QDateList &dates);
    void fillContactInfo(LogEntry *entry, const QContact &contact);
    void clearContactInfo(LogEntry *entry);
    void appendEvents(const Tpl::EventPtrList &events);

    virtual LogEntry *createEntry(const Tpl::EventPtr &event);
    virtual void handleEntities(const Tpl::EntityPtrList &entities);
    virtual void handleDates(const Tpl::EntityPtr &entity, const Tpl::QDateList &dates);
    virtual void handleEvents(const Tpl::EventPtrList &events);

protected slots:
    void onPendingEntitiesFinished(Tpl::PendingOperation *op);
    void onPendingDatesFinished(Tpl::PendingOperation *op);
    void onPendingEventsFinished(Tpl::PendingOperation *op);

    // QContactManager related slots
    void onContactsAdded(const QList<QContactLocalId> &contactIds);
    void onContactsChanged(const QList<QContactLocalId> &contactIds);
    void onContactsRemoved(const QList<QContactLocalId> &contactIds);

private:
    QList<LogEntry*> mLogEntries;
    QtMobility::QContactManager *mContactManager;
    Tpl::EventTypeMask mType;
};

#endif // CALLLOGMODEL_H
