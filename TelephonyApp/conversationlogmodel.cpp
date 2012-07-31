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

#include "chatmanager.h"
#include "conversationlogmodel.h"
#include "contactmodel.h"
#include <TelepathyLoggerQt4/Event>
#include <TelepathyLoggerQt4/TextEvent>
#include <TelepathyLoggerQt4/Entity>

QVariant ConversationLogEntry::data(int role) const
{
    switch (role) {
    case ConversationLogModel::Message:
        return message;
    case ConversationLogModel::ThreadId:
        return threadId;
    case ConversationLogModel::UnreadCount:
        return unreadCount;
    default:
        return LogEntry::data(role);
    }
}

ConversationLogModel::ConversationLogModel(QObject *parent) :
    AbstractLoggerModel(parent)
{
    // set the role names
    QHash<int, QByteArray> roles = roleNames();
    roles[Message] = "message";
    roles[ThreadId] = "threadId";
    roles[UnreadCount] = "unreadCount";
    setRoleNames(roles);

    connect(ChatManager::instance(),
            SIGNAL(unreadMessagesChanged(const QString&)),
            SLOT(onUnreadMessagesChanged(const QString&)));

    fetchLog(Tpl::EventTypeMaskText);
}

void ConversationLogModel::onMessageReceived(const QString &number, const QString &message)
{
    updateLatestMessage(number, message, true);
}

void ConversationLogModel::onMessageSent(const QString &number, const QString &message)
{
    updateLatestMessage(number, message, false);
}

void ConversationLogModel::onUnreadMessagesChanged(const QString &number)
{
    // find if we have a conversation to the number already
    int count = mLogEntries.count();
    for (int i = 0; i < count; ++i) {
        LogEntry *entry = mLogEntries[i];
        if (ContactModel::instance()->comparePhoneNumbers(entry->phoneNumber, number)) {
            ConversationLogEntry *conversationEntry = dynamic_cast<ConversationLogEntry*>(entry);
            if (!conversationEntry) {
                continue;
            }
            conversationEntry->unreadCount = ChatManager::instance()->unreadMessages(number);
            emit dataChanged(index(i, 0), index(i,0));
        }
    }
}

LogEntry *ConversationLogModel::createEntry(const Tpl::EventPtr &event)
{
    ConversationLogEntry *entry = new ConversationLogEntry();
    Tpl::TextEventPtr textEvent = event.dynamicCast<Tpl::TextEvent>();

    if (!textEvent) {
        qWarning() << "The event" << event << "is not a Tpl::TextEvent!";
    }

    entry->message = textEvent->message();
    entry->threadId = threadIdFromIdentifier(textEvent->receiver()->identifier());
    entry->phoneNumber = textEvent->receiver()->alias();
    entry->unreadCount = ChatManager::instance()->unreadMessages(entry->phoneNumber);

    return entry;
}

void ConversationLogModel::handleEntities(const Tpl::EntityPtrList &entities)
{
    Tpl::EntityPtrList filteredEntities;

    // For messages we only use room entities in telepathy-ufa
    // FIXME: this is not generic for all connection managers
    Q_FOREACH(const Tpl::EntityPtr &entity, entities) {
        if (entity->entityType() == Tpl::EntityTypeRoom) {
            filteredEntities << entity;
        }
    }

    AbstractLoggerModel::handleEntities(filteredEntities);
}

void ConversationLogModel::handleDates(const Tpl::EntityPtr &entity, const Tpl::QDateList &dates)
{
    if (!dates.count()) {
        return;
    }
    QDate newestDate = dates.first();

    // search for the newest available date
    Q_FOREACH(const QDate &date, dates) {
        if (date > newestDate) {
            newestDate = date;
        }
    }

    requestEventsForDates(entity, Tpl::QDateList() << newestDate);
}

void ConversationLogModel::handleEvents(const Tpl::EventPtrList &events)
{
    if (!events.count()) {
        return;
    }

    // This function is going to be called once per entity, so we don't need to check for the entity
    // at this point

    Tpl::TextEventPtr newestEvent;
    bool found = false;

    // search for the newest message
    Q_FOREACH(const Tpl::EventPtr &event, events) {
        const Tpl::TextEventPtr textEvent = event.dynamicCast<Tpl::TextEvent>();
        if (!textEvent) {
            continue;
        }

        // check if the message is an incoming one and if it was read already
        // and if it was not, ignore it
        bool incoming = textEvent->sender()->entityType() != Tpl::EntityTypeSelf;
        if (incoming && textEvent->editTimestamp().toTime_t() == 0) {
            continue;
        }

        if (!found || textEvent->timestamp() > newestEvent->timestamp()) {
            found = true;
            newestEvent = textEvent;
        }
    }

    // if no read message was found, exit the function
    if (!found) {
        return;
    }

    // after finding the latest message, check if we have an entry for it already
    // and if we do, just update it
    ConversationLogEntry *entry = findEntry(threadIdFromIdentifier(newestEvent->receiver()->identifier()),
                                            newestEvent->receiver()->alias());
    if (entry) {
        if (entry->timestamp >= newestEvent->timestamp()) {
            return;
        }

        entry->timestamp = newestEvent->timestamp();
        entry->threadId = threadIdFromIdentifier(newestEvent->receiver()->identifier());
        entry->message = newestEvent->message();
        entry->incoming = newestEvent->sender()->entityType() != Tpl::EntityTypeSelf;
        QModelIndex index = indexFromEntry(entry);
        if (index.isValid()) {
            emit dataChanged(index, index);
        }
    } else {
        // if we didn't find an entry, append the message
        appendEvents(Tpl::EventPtrList() << newestEvent);
    }
}

void ConversationLogModel::updateLatestMessage(const QString &number, const QString &message, bool incoming)
{
    int count = mLogEntries.count();
    for(int i = 0; i < count; ++i) {
        ConversationLogEntry *entry = dynamic_cast<ConversationLogEntry*>(mLogEntries[i]);
        if (!entry) {
            continue;
        }

        if (ContactModel::instance()->comparePhoneNumbers(entry->phoneNumber, number)) {
            entry->timestamp = QDateTime::currentDateTime();
            entry->message = message;
            entry->incoming = incoming;
            entry->unreadCount = ChatManager::instance()->unreadMessages(number);
            emit dataChanged(index(i,0), index(i,0));
            return;
        }
    }

    // if we reach this point, there is a new conversation, so create the item
    ConversationLogEntry *entry = new ConversationLogEntry();
    entry->timestamp = QDateTime::currentDateTime();
    entry->incoming = incoming;
    entry->message = message;
    entry->phoneNumber = number;
    entry->customId = ContactModel::instance()->customIdFromPhoneNumber(number);
    entry->unreadCount = ChatManager::instance()->unreadMessages(number);

    // set the alias to the phone number as a fallback value in case the contact is not found.
    entry->contactAlias = number;

    ContactEntry *contact = ContactModel::instance()->contactFromPhoneNumber(number);
    if (contact) {
        fillContactInfo(entry, contact);
    }

    appendEntry(entry);
}

ConversationLogEntry *ConversationLogModel::findEntry(const QString &threadId, const QString &phoneNumber)
{
    Q_FOREACH(LogEntry *logEntry, mLogEntries) {
        ConversationLogEntry *entry = dynamic_cast<ConversationLogEntry*>(logEntry);
        if (!entry) {
            continue;
        }
        if (entry->threadId == threadId ||
            ContactModel::instance()->comparePhoneNumbers(entry->phoneNumber, phoneNumber)) {
            return entry;
        }
    }

    return 0;
}
