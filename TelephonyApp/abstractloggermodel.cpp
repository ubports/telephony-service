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

#include "abstractloggermodel.h"
#include "telepathyhelper.h"
#include "contactentry.h"
#include "contactmodel.h"
#include <TelepathyLoggerQt4/LogManager>
#include <TelepathyLoggerQt4/PendingDates>
#include <TelepathyLoggerQt4/PendingEntities>
#include <TelepathyLoggerQt4/PendingEvents>
#include <TelepathyLoggerQt4/Entity>
#include <TelepathyLoggerQt4/Event>

QVariant LogEntry::data(int role) const
{
    switch (role) {
    case AbstractLoggerModel::ContactId:
        return contactId;
    case AbstractLoggerModel::CustomId:
        return customId;
    case AbstractLoggerModel::ContactAlias:
    case Qt::DisplayRole:
        return contactAlias;
    case AbstractLoggerModel::Avatar:
    case Qt::DecorationRole:
        return avatar;
    case AbstractLoggerModel::PhoneNumber:
        return phoneNumber;
    case AbstractLoggerModel::PhoneType:
        return phoneType;
    case AbstractLoggerModel::Timestamp:
        return timestamp;
    case AbstractLoggerModel::Incoming:
        return incoming;
    default:
        return QVariant();
    }
}

AbstractLoggerModel::AbstractLoggerModel(QObject *parent) :
    QAbstractListModel(parent), mType(Tpl::EventTypeMaskAny), mLogManager(Tpl::LogManager::instance())
{
    // set the role names
    QHash<int, QByteArray> roles;
    roles[ContactId] = "contactId";
    roles[CustomId] = "customId";
    roles[ContactAlias] = "contactAlias";
    roles[Avatar] = "avatar";
    roles[PhoneNumber] = "phoneNumber";
    roles[PhoneType] = "phoneType";
    roles[Timestamp] = "timestamp";
    roles[Incoming] = "incoming";
    setRoleNames(roles);

    connect(ContactModel::instance(),
            SIGNAL(contactAdded(ContactEntry*)),
            SLOT(onContactAdded(ContactEntry*)));
    connect(ContactModel::instance(),
            SIGNAL(contactChanged(ContactEntry*)),
            SLOT(onContactChanged(ContactEntry*)));
    connect(ContactModel::instance(),
            SIGNAL(contactRemoved(const QString&)),
            SLOT(onContactRemoved(const QString&)));
}

int AbstractLoggerModel::rowCount(const QModelIndex &parent) const
{
    // no child items
    if (parent.isValid()) {
        return 0;
    }

    return mLogEntries.count();
}

QVariant AbstractLoggerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= mLogEntries.count()) {
        return QVariant();
    }

    return mLogEntries[index.row()]->data(role);
}

QString AbstractLoggerModel::phoneNumberFromId(const QString &id) const
{
    QStringList splittedId = id.split(":");
    if (splittedId.count() == 2) {
        return splittedId[1];
    } else {
        qWarning() << "The ID from logger is not using the format contactId:phoneNumber" << id;
    }

    return id;
}

QString AbstractLoggerModel::threadIdFromIdentifier(const QString &id) const
{
    // that is basically the same method, but better to use the right name
    return phoneNumberFromId(id);
}

QString AbstractLoggerModel::customIdentifierFromId(const QString &id) const
{
    QStringList splittedId = id.split(":");
    if (splittedId.count() == 2) {
        return splittedId[0];
    } else {
        qWarning() << "The ID from logger is not using the format contactId:phoneNumber" << id;
    }

    return id;
}

void AbstractLoggerModel::fetchLog(Tpl::EventTypeMask type)
{
    Tpl::PendingEntities *pendingEntities = mLogManager->queryEntities(TelepathyHelper::instance()->account());

    // store the type for the event fetching stage
    mType = type;

    /* Fetching the log work like this:
       - Start by fetching the entities from the log
       - Once you get the entities, fetch the available dates
       - After you get the dates, fetch the events themselves
     */
    connect(pendingEntities,
            SIGNAL(finished(Tpl::PendingOperation*)),
            SLOT(onPendingEntitiesFinished(Tpl::PendingOperation*)));
    mActiveOperations.append(pendingEntities);
}

void AbstractLoggerModel::requestDatesForEntities(const Tpl::EntityPtrList &entities)
{
    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    foreach(Tpl::EntityPtr entity, entities) {
        Tpl::PendingDates *pendingDates = mLogManager->queryDates(account, entity, mType);

        connect(pendingDates,
                SIGNAL(finished(Tpl::PendingOperation*)),
                SLOT(onPendingDatesFinished(Tpl::PendingOperation*)));
        mActiveOperations.append(pendingDates);
    }
}

void AbstractLoggerModel::requestEventsForDates(const Tpl::EntityPtr &entity, const Tpl::QDateList &dates)
{
    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    foreach(QDate date, dates) {
        Tpl::PendingEvents *pendingEvents = mLogManager->queryEvents(account, entity, mType, date);
        connect(pendingEvents,
                SIGNAL(finished(Tpl::PendingOperation*)),
                SLOT(onPendingEventsFinished(Tpl::PendingOperation*)));
        mActiveOperations.append(pendingEvents);
    }
}

void AbstractLoggerModel::fillContactInfo(LogEntry *entry, ContactEntry *contact)
{
    entry->contactId = contact->id();
    entry->customId = contact->customId();
    entry->avatar = contact->avatar();
    entry->contactAlias = contact->displayLabel();
}

void AbstractLoggerModel::clearContactInfo(LogEntry *entry)
{
    entry->avatar = "";
    entry->contactId = "";
    entry->customId = "";
    entry->contactAlias = "";
}

void AbstractLoggerModel::appendEvents(const Tpl::EventPtrList &events)
{
    // add the events to the list
    beginInsertRows(QModelIndex(), mLogEntries.count(), (mLogEntries.count() + events.count()-1));
    foreach(Tpl::EventPtr event, events) {
        LogEntry *entry = createEntry(event);
        if (!entry) {
            continue;
        }
        entry->incoming = (event->sender()->entityType() != Tpl::EntityTypeSelf);
        entry->timestamp = event->timestamp();

        Tpl::EntityPtr remoteEntity = entry->incoming ? event->sender() : event->receiver();
        parseEntityId(remoteEntity, entry);

        // set the alias from the entity as a fallback value in case the contact is not found.
        entry->contactAlias = remoteEntity->alias();

        ContactEntry *contact = ContactModel::instance()->contactFromCustomId(entry->customId);
        if (contact) {
            // if more than one contact matches, use the first one
            fillContactInfo(entry, contact);
        }

        mLogEntries.append(entry);
    }
    endInsertRows();
}

void AbstractLoggerModel::appendEntry(LogEntry *entry)
{
    beginInsertRows(QModelIndex(), mLogEntries.count(), mLogEntries.count());
    mLogEntries.append(entry);
    endInsertRows();
}

void AbstractLoggerModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, mLogEntries.count()-1);
    qDeleteAll(mLogEntries);
    mLogEntries.clear();
    endRemoveRows();
}

void AbstractLoggerModel::invalidateRequests()
{
    // clear the model
    clear();

    // and invalidate all pending requests
    mActiveOperations.clear();
}

bool AbstractLoggerModel::validateRequest(Tpl::PendingOperation *op)
{
    // if invalidateRequests() was called, the *op is not going to be on the list anymore
    // so we consider it invalid
    int index = mActiveOperations.indexOf(op);

    if (index >= 0) {
        mActiveOperations.removeAt(index);
        return true;
    }

    return false;
}

LogEntry *AbstractLoggerModel::createEntry(const Tpl::EventPtr &event)
{
    Q_UNUSED(event);
    return new LogEntry();
}

void AbstractLoggerModel::handleEntities(const Tpl::EntityPtrList &entities)
{
    // just request the dates for all the entities
    requestDatesForEntities(entities);
}

void AbstractLoggerModel::handleDates(const Tpl::EntityPtr &entity, const Tpl::QDateList &dates)
{
    // just request all events by default
    requestEventsForDates(entity, dates);
}

void AbstractLoggerModel::handleEvents(const Tpl::EventPtrList &events)
{
    // we have to clear the cache right before
    // adding new items to the model or we
    // might have duplicated data if we receive messages while
    // fetching
    clear();
    // just add all the events to the list
    appendEvents(events);
}

void AbstractLoggerModel::onPendingEntitiesFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingEntities *pe = qobject_cast<Tpl::PendingEntities*>(op);
    if (!pe) {
        return;
    }

    handleEntities(pe->entities());
}

void AbstractLoggerModel::onPendingDatesFinished(Tpl::PendingOperation *op)
{
    // check if the request is still valid
    if (!validateRequest(op)) {
        return;
    }

    Tpl::PendingDates *pd = qobject_cast<Tpl::PendingDates*>(op);
    if (!pd) {
        return;
    }

    handleDates(pd->entity(), pd->dates());
}

void AbstractLoggerModel::onPendingEventsFinished(Tpl::PendingOperation *op)
{
    // check if the request is still valid
    if (!validateRequest(op)) {
        return;
    }

    Tpl::PendingEvents *pe = qobject_cast<Tpl::PendingEvents*>(op);
    if (!pe) {
        return;
    }

    handleEvents(pe->events());
}

void AbstractLoggerModel::onContactAdded(ContactEntry *contact)
{
    // now we need to iterate over the events to look for contacts matching
    int count = mLogEntries.count();
    QString customId = contact->customId();
    for (int i = 0; i < count; ++i) {
        LogEntry *entry = mLogEntries[i];
        if (entry->customId == customId) {
            fillContactInfo(entry, contact);
            emit dataChanged(index(i,0), index(i,0));
        }
    }
}

void AbstractLoggerModel::onContactChanged(ContactEntry *contact)
{
    int count = mLogEntries.count();
    QString customId = contact->customId();
    for (int i = 0; i < count; ++i) {
        LogEntry *entry = mLogEntries[i];
        if (entry->customId == customId) {
            fillContactInfo(entry, contact);
            emit dataChanged(index(i, 0), index(i, 0));
        }
    }
}

void AbstractLoggerModel::onContactRemoved(const QString &contactId)
{
    int count = mLogEntries.count();
    for (int i = 0; i < count; ++i) {
        LogEntry *entry = mLogEntries[i];
        if (entry->contactId == contactId) {
            clearContactInfo(entry);
            emit dataChanged(index(i,0), index(i,0));
        }
    }
}

void AbstractLoggerModel::parseEntityId(const Tpl::EntityPtr &entity, LogEntry *entry)
{
    entry->customId = customIdentifierFromId(entity->identifier());
    if (entity->entityType() == Tpl::EntityTypeRoom) {
        entry->phoneNumber = entity->alias();
    } else {
        entry->phoneNumber = phoneNumberFromId(entity->identifier());
    }
}

