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

#include "messagelogmodel.h"
#include "contactmodel.h"
#include <TelepathyLoggerQt4/Event>
#include <TelepathyLoggerQt4/TextEvent>
#include <TelepathyLoggerQt4/Entity>

QVariant MessageLogEntry::data(int role) const
{
    switch (role) {
    case MessageLogModel::Message:
        return message;
    case MessageLogModel::Date:
        return timestamp.date().toString(Qt::DefaultLocaleLongDate);
    case MessageLogModel::ThreadId:
        return threadId;
    default:
        return LogEntry::data(role);
    }
}

MessageLogModel::MessageLogModel(QObject *parent) :
    AbstractLoggerModel(parent)
{
    // set the role names
    QHash<int, QByteArray> roles = roleNames();
    roles[Message] = "message";
    roles[Date] = "date";
    roles[ThreadId] = "threadId";
    setRoleNames(roles);

    fetchLog(Tpl::EventTypeMaskText, EntityTypeList() << Tpl::EntityTypeRoom);
}

void MessageLogModel::appendMessage(const QString &number, const QString &message, bool incoming, const QDateTime &timestamp)
{
    MessageLogEntry *entry = new MessageLogEntry();
    entry->incoming = incoming;
    entry->phoneNumber = number;
    entry->message = message;
    entry->timestamp = timestamp;
    ContactEntry *contact = ContactModel::instance()->contactFromPhoneNumber(number);
    if (contact) {
        fillContactInfo(entry, contact);
    }
    appendEntry(entry);
}

void MessageLogModel::onMessageReceived(const QString &number, const QString &message, const QDateTime &timestamp)
{
    appendMessage(number, message, true, timestamp);
    // force the proxy model to reset to workaround a ListView bug
    emit resetView();
}

void MessageLogModel::onMessageSent(const QString &number, const QString &message)
{
    appendMessage(number, message, false);
    // force the proxy model to reset to workaround a ListView bug
    emit resetView();
}

LogEntry *MessageLogModel::createEntry(const Tpl::EventPtr &event)
{
    MessageLogEntry *entry = new MessageLogEntry();
    Tpl::TextEventPtr textEvent = event.dynamicCast<Tpl::TextEvent>();

    if (!textEvent) {
        qWarning() << "The event" << event << "is not a Tpl::TextEvent!";
    }

    entry->message = textEvent->message();
    entry->threadId = threadIdFromIdentifier(textEvent->receiver()->identifier());
    return entry;
}

void MessageLogModel::handleEvents(const Tpl::EventPtrList &events)
{
    Tpl::EventPtrList filteredEvents;

    Q_FOREACH(const Tpl::EventPtr &event, events) {
        const Tpl::TextEventPtr textEvent = event.dynamicCast<Tpl::TextEvent>();
        if (!textEvent) {
            continue;
        }

        // if the edit timestamp is set, the message was already read and can
        // be appended to the model.
        // Also, if the message is outgoing, it should also be appeneded to the model
        bool outgoing = textEvent->sender()->entityType() == Tpl::EntityTypeSelf;
        if (outgoing || textEvent->editTimestamp().toTime_t() > 0) {
            filteredEvents.append(event);
        }
    }

    AbstractLoggerModel::handleEvents(filteredEvents);
}

