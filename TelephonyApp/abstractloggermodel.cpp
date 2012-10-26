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
#include <TelepathyLoggerQt/LogManager>
#include <TelepathyLoggerQt/PendingDates>
#include <TelepathyLoggerQt/PendingEntities>
#include <TelepathyLoggerQt/PendingEvents>
#include <TelepathyLoggerQt/Entity>
#include <TelepathyLoggerQt/Event>
#include <QContact>
#include <QContactPhoneNumber>

AbstractLoggerModel::AbstractLoggerModel(QObject *parent) :
    ConversationFeedModel(parent), mType(Tpl::EventTypeMaskAny), mLogManager(Tpl::LogManager::instance())
{
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

void AbstractLoggerModel::populate()
{
    fetchLog();
}

void AbstractLoggerModel::fetchLog(Tpl::EventTypeMask type, EntityTypeList entityTypes)
{
    Tpl::PendingEntities *pendingEntities = mLogManager->queryEntities(TelepathyHelper::instance()->account());

    if (entityTypes.isEmpty()) {
        entityTypes << Tpl::EntityTypeContact
                    << Tpl::EntityTypeRoom
                    << Tpl::EntityTypeSelf
                    << Tpl::EntityTypeUnknown;
    }

    // store the type and the entity types for the event fetching stage
    mType = type;
    mEntityTypes = entityTypes;

    /* Fetching the log work like this:
       - Start by fetching the entities from the log
       - Once you get the entities, fetch the available dates
       - After you get the dates, fetch the events themselves
     */
    connect(pendingEntities,
            SIGNAL(finished(Tpl::PendingOperation*)),
            SLOT(onPendingEntitiesFinished(Tpl::PendingOperation*)),
            Qt::DirectConnection);
    mActiveOperations.append(pendingEntities);
}

void AbstractLoggerModel::requestDatesForEntities(const Tpl::EntityPtrList &entities)
{
    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    Q_FOREACH(Tpl::EntityPtr entity, entities) {
        Tpl::PendingDates *pendingDates = mLogManager->queryDates(account, entity, mType);

        connect(pendingDates,
                SIGNAL(finished(Tpl::PendingOperation*)),
                SLOT(onPendingDatesFinished(Tpl::PendingOperation*)),
                Qt::DirectConnection);
        mActiveOperations.append(pendingDates);
    }
}

void AbstractLoggerModel::requestEventsForDates(const Tpl::EntityPtr &entity, const Tpl::QDateList &dates)
{
    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    Q_FOREACH(QDate date, dates) {
        Tpl::PendingEvents *pendingEvents = mLogManager->queryEvents(account, entity, mType, date);
        connect(pendingEvents,
                SIGNAL(finished(Tpl::PendingOperation*)),
                SLOT(onPendingEventsFinished(Tpl::PendingOperation*)),
                Qt::DirectConnection);
        mActiveOperations.append(pendingEvents);
    }
}

void AbstractLoggerModel::fillContactInfo(LoggerItem *entry, ContactEntry *contact)
{
    entry->setContactId(contact->id().toString());
    entry->setContactAvatar(contact->avatar());
    entry->setContactAlias(contact->displayLabel());
}

void AbstractLoggerModel::clearContactInfo(LoggerItem *entry)
{
    entry->setContactAvatar(QUrl(""));
    entry->setContactId("");
    entry->setContactAlias("");
}

bool AbstractLoggerModel::checkNonStandardNumbers(LoggerItem *entry)
{
    bool changed = false;

    if (!entry) {
        return changed;
    }

    if (entry->phoneNumber() == QLatin1String("-2")) {
        entry->setContactAlias(QLatin1String("Private number"));
        entry->setPhoneNumber(QLatin1String("-"));
        changed = true;
    } else if (entry->phoneNumber() == QLatin1String("-1") || entry->phoneNumber() == QLatin1String("#")) {
        entry->setContactAlias(QLatin1String("Unknown number"));
        entry->setPhoneNumber(QLatin1String("-"));
        changed = true;
    }

    return changed;
}

void AbstractLoggerModel::appendEvents(const Tpl::EventPtrList &events)
{
    // add the events to the list
    Q_FOREACH(Tpl::EventPtr event, events) {
        LoggerItem *entry = createEntry(event);
        if (!entry) {
            continue;
        }
        entry->setIncoming(event->sender()->entityType() != Tpl::EntityTypeSelf);
        entry->setTimestamp(event->timestamp());

        Tpl::EntityPtr remoteEntity = entry->incoming() ? event->sender() : event->receiver();
        entry->setPhoneNumber(remoteEntity->identifier());

        if (!checkNonStandardNumbers(entry)) {
            // set the alias from the entity as a fallback value in case the contact is not found.
            entry->setContactAlias(remoteEntity->alias());

            ContactEntry *contact = ContactModel::instance()->contactFromPhoneNumber(entry->phoneNumber());
            if (contact) {
                fillContactInfo(entry, contact);
            }
        }

        addItem(entry);
    }
    Q_EMIT resetView();
}

void AbstractLoggerModel::appendEntry(LoggerItem *entry)
{
    addItem(entry);
}

void AbstractLoggerModel::clear()
{
    ConversationFeedModel::clear();
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

void AbstractLoggerModel::updateLogForContact(ContactEntry *contactEntry)
{
    // now we need to iterate over the events to look for contacts matching
    int count = mItems.count();
    for (int i = 0; i < count; ++i) {
        LoggerItem *entry = dynamic_cast<LoggerItem*>(mLogEntries[i]);
        // check if any of the contact's phone numbers match
        Q_FOREACH(const QContactPhoneNumber &number, contactEntry->contact().details<QContactPhoneNumber>()) {
            if (ContactModel::comparePhoneNumbers(entry->phoneNumber(), number.number())) {
                fillContactInfo(entry, contactEntry);
                Q_EMIT dataChanged(index(i,0), index(i,0));
                continue;
            }
        }
    }
}

LoggerItem *AbstractLoggerModel::createEntry(const Tpl::EventPtr &event)
{
    Q_UNUSED(event);
    return new LoggerItem();
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
    // just add all the events to the list
    appendEvents(events);
}

void AbstractLoggerModel::onPendingEntitiesFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingEntities *pe = qobject_cast<Tpl::PendingEntities*>(op);
    if (!pe) {
        return;
    }

    // filter out entities we are not interested in
    Tpl::EntityPtrList filteredEntitites;
    Q_FOREACH(const Tpl::EntityPtr &entity, pe->entities()) {
        if (mEntityTypes.contains(entity->entityType())) {
            filteredEntitites << entity;
        }
    }

    handleEntities(filteredEntitites);
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
    updateLogForContact(contact);
}

void AbstractLoggerModel::onContactChanged(ContactEntry *contact)
{
    updateLogForContact(contact);
}

void AbstractLoggerModel::onContactRemoved(const QString &contactId)
{
    int count = mItems.count();
    for (int i = 0; i < count; ++i) {
        LoggerItem *entry = dynamic_cast<LoggerItem*>(mItems[i]);
        if (entry->contactId() == contactId) {
            clearContactInfo(entry);
            Q_EMIT dataChanged(index(i,0), index(i,0));
        }
    }
}
