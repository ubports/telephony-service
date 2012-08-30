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
#include <TelepathyLoggerQt4/LogManager>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QUrl>

class ContactEntry;

typedef QList<Tpl::EntityType> EntityTypeList;

class LogEntry {
public:
    virtual QVariant data(int role) const;

    // for simplicity keep the members as public
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

    explicit AbstractLoggerModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

Q_SIGNALS:
    void resetView();
    
protected:
    QString phoneNumberFromId(const QString &id) const;
    QString threadIdFromIdentifier(const QString &id) const;
    void fetchLog(Tpl::EventTypeMask type = Tpl::EventTypeMaskAny, EntityTypeList entityTypes = EntityTypeList());
    void requestDatesForEntities(const Tpl::EntityPtrList &entities);
    void requestEventsForDates(const Tpl::EntityPtr &entity, const Tpl::QDateList &dates);
    void fillContactInfo(LogEntry *entry, ContactEntry *contact);
    void clearContactInfo(LogEntry *entry);
    void appendEvents(const Tpl::EventPtrList &events);
    void appendEntry(LogEntry *entry);
    void clear();
    void invalidateRequests();
    bool validateRequest(Tpl::PendingOperation *op);
    QModelIndex indexFromEntry(LogEntry *entry) const;
    void updateLogForContact(ContactEntry *contactEntry);

    virtual LogEntry *createEntry(const Tpl::EventPtr &event);
    virtual void handleEntities(const Tpl::EntityPtrList &entities);
    virtual void handleDates(const Tpl::EntityPtr &entity, const Tpl::QDateList &dates);
    virtual void handleEvents(const Tpl::EventPtrList &events);
    bool checkNonStandardNumbers(LogEntry *entry);

protected Q_SLOTS:
    void onPendingEntitiesFinished(Tpl::PendingOperation *op);
    void onPendingDatesFinished(Tpl::PendingOperation *op);
    void onPendingEventsFinished(Tpl::PendingOperation *op);

    // ContactModel related slots
    void onContactAdded(ContactEntry *contact);
    void onContactChanged(ContactEntry *contact);
    void onContactRemoved(const QString &contactId);

protected:
    void parseEntityId(const Tpl::EntityPtr &entity, LogEntry *entry);

    Tpl::LogManagerPtr mLogManager;
    QList<LogEntry*> mLogEntries;
    Tpl::EventTypeMask mType;
    EntityTypeList mEntityTypes;
    QList<Tpl::PendingOperation*> mActiveOperations;
};

#endif // CALLLOGMODEL_H
