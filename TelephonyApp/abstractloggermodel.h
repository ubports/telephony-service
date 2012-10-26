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
#include <TelepathyLoggerQt/PendingOperation>
#include <TelepathyLoggerQt/Types>
#include <TelepathyLoggerQt/LogManager>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QUrl>

#include "conversationfeedmodel.h"
#include "conversationfeeditem.h"

class ContactEntry;

typedef QList<Tpl::EntityType> EntityTypeList;

class LoggerItem : public ConversationFeedItem {
    Q_OBJECT
    Q_PROPERTY(QString phoneNumber READ phoneNumber WRITE setPhoneNumber NOTIFY phoneNumberChanged)

public:
    void setPhoneNumber(const QString &phone) { mPhoneNumber = phone; Q_EMIT contactIdChanged(); };
    QString phoneNumber() { return mPhoneNumber; };
Q_SIGNALS:
    void phoneNumberChanged();
private:
    QString mPhoneNumber;
};

class AbstractLoggerModel : public ConversationFeedModel
{
    Q_OBJECT
public:
    explicit AbstractLoggerModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    virtual void populate();

Q_SIGNALS:
    void resetView();
    
protected:
    void fetchLog(Tpl::EventTypeMask type = Tpl::EventTypeMaskAny, EntityTypeList entityTypes = EntityTypeList());
    void requestDatesForEntities(const Tpl::EntityPtrList &entities);
    void requestEventsForDates(const Tpl::EntityPtr &entity, const Tpl::QDateList &dates);
    void fillContactInfo(LoggerItem *entry, ContactEntry *contact);
    void clearContactInfo(LoggerItem *entry);
    void appendEvents(const Tpl::EventPtrList &events);
    void appendEntry(LoggerItem *entry);
    void invalidateRequests();
    bool validateRequest(Tpl::PendingOperation *op);
    void updateLogForContact(ContactEntry *contactEntry);

    virtual LoggerItem *createEntry(const Tpl::EventPtr &event);
    virtual void handleEntities(const Tpl::EntityPtrList &entities);
    virtual void handleDates(const Tpl::EntityPtr &entity, const Tpl::QDateList &dates);
    virtual void handleEvents(const Tpl::EventPtrList &events);
    bool checkNonStandardNumbers(LoggerItem *entry);

protected Q_SLOTS:
    void onPendingEntitiesFinished(Tpl::PendingOperation *op);
    void onPendingDatesFinished(Tpl::PendingOperation *op);
    void onPendingEventsFinished(Tpl::PendingOperation *op);

    // ContactModel related slots
    void onContactAdded(ContactEntry *contact);
    void onContactChanged(ContactEntry *contact);
    void onContactRemoved(const QString &contactId);

protected:
    Tpl::LogManagerPtr mLogManager;
    QList<LoggerItem*> mLogEntries;
    Tpl::EventTypeMask mType;
    EntityTypeList mEntityTypes;
    QList<Tpl::PendingOperation*> mActiveOperations;
};

#endif // CALLLOGMODEL_H
