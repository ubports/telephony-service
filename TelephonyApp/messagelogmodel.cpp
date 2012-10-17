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
#include <TelepathyLoggerQt/Event>
#include <TelepathyLoggerQt/TextEvent>
#include <TelepathyLoggerQt/Entity>

QVariant MessageLogEntry::data(int role) const
{
    switch (role) {
    case MessageLogModel::Message:
        return message;
    case MessageLogModel::Date:
        return timestamp.date().toString(Qt::DefaultLocaleLongDate);
    case MessageLogModel::MessageId:
        return messageId;
    case MessageLogModel::IsLatest:
        return isLatest;
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
    roles[MessageId] = "messageId";
    roles[IsLatest] = "isLatest";
    setRoleNames(roles);
}

void MessageLogModel::appendMessage(const QString &number,
                                    const QString &message,
                                    bool incoming,
                                    const QDateTime &timestamp,
                                    const QString &messageId)
{
    MessageLogEntry *entry = new MessageLogEntry();
    entry->incoming = incoming;
    entry->phoneNumber = number;
    entry->message = message;
    entry->timestamp = timestamp;
    entry->messageId = messageId;
    // set the alias to the phone number as a fallback in case the contact is not known
    entry->contactAlias = number;

    ContactEntry *contact = ContactModel::instance()->contactFromPhoneNumber(number);
    if (contact) {
        fillContactInfo(entry, contact);
    }
    appendEntry(entry);

    updateLatestMessages(number);
}

void MessageLogModel::populate()
{
     fetchLog(Tpl::EventTypeMaskText);
}

void MessageLogModel::onMessageReceived(const QString &number,
                                        const QString &message,
                                        const QDateTime &timestamp,
                                        const QString &messageId)
{
    // check if the message is already in the model (it might have been loaded from logger)
    if (!messageId.isEmpty() && messageById(messageId) != 0) {
        return;
    }
    appendMessage(number, message, true, timestamp, messageId);
    // force the proxy model to reset to workaround a ListView bug
    Q_EMIT resetView();
}

void MessageLogModel::onMessageSent(const QString &number, const QString &message)
{
    appendMessage(number, message, false);
    // force the proxy model to reset to workaround a ListView bug
    Q_EMIT resetView();
}

LogEntry *MessageLogModel::createEntry(const Tpl::EventPtr &event)
{
    MessageLogEntry *entry = new MessageLogEntry();
    Tpl::TextEventPtr textEvent = event.dynamicCast<Tpl::TextEvent>();

    if (!textEvent) {
        qWarning() << "The event" << event << "is not a Tpl::TextEvent!";
    }

    entry->messageId = textEvent->messageToken();
    entry->message = textEvent->message();
    entry->isLatest = false;
    return entry;
}

void MessageLogModel::handleEvents(const Tpl::EventPtrList &events)
{
    Tpl::EventPtrList filteredEvents;
    QStringList phoneNumbers;

    Q_FOREACH(const Tpl::EventPtr &event, events) {
        const Tpl::TextEventPtr textEvent = event.dynamicCast<Tpl::TextEvent>();
        if (!textEvent) {
            continue;
        }

        // check if the message is already in the model (it might have been added by telepathy)
        if (!textEvent->messageToken().isEmpty() && messageById(textEvent->messageToken()) != 0) {
            continue;
        }

        // if the edit timestamp is set, the message was already read and can
        // be appended to the model.
        // Also, if the message is outgoing, it should also be appeneded to the model
        bool outgoing = textEvent->sender()->entityType() == Tpl::EntityTypeSelf;
        filteredEvents.append(event);
        // add the number to the phone numbers list
        QString phoneNumber = outgoing ? textEvent->receiver()->identifier() :
                                         textEvent->sender()->identifier();
        if (!phoneNumbers.contains(phoneNumber)) {
            phoneNumbers.append(phoneNumber);
        }
    }

    AbstractLoggerModel::handleEvents(filteredEvents);

    // update the latest messages for the numbers we loaded
    Q_FOREACH (const QString &phoneNumber, phoneNumbers) {
        updateLatestMessages(phoneNumber);
    }
}

void MessageLogModel::updateLatestMessages(const QString &phoneNumber)
{
    if (mLogEntries.count() == 0) {
        return;
    }

    MessageLogEntry *latestEntry = 0;

    // go through the list of messages trying to find the latest one, and reset the latest flag on other items
    Q_FOREACH (LogEntry *entry, mLogEntries) {
        if (!ContactModel::instance()->comparePhoneNumbers(entry->phoneNumber, phoneNumber)) {
            continue;
        }

        MessageLogEntry *messageEntry = dynamic_cast<MessageLogEntry*>(entry);

        // reset the isLatest flag
        if (messageEntry->isLatest) {
            QModelIndex index = indexFromEntry(messageEntry);
            messageEntry->isLatest = false;
            Q_EMIT dataChanged(index, index);
        }

        if (!latestEntry) {
            latestEntry = messageEntry;
            continue;
        }

        if (messageEntry->timestamp > latestEntry->timestamp) {
            latestEntry = messageEntry;
        }
    }

    if (!latestEntry) {
        return;
    }

    // after finding the latest one, mark it as being so
    QModelIndex index = indexFromEntry(latestEntry);
    latestEntry->isLatest = true;
    Q_EMIT dataChanged(index, index);
}

MessageLogEntry *MessageLogModel::messageById(const QString &messageId)
{
    Q_FOREACH(LogEntry *entry, mLogEntries) {
        if (entry->data(MessageId).toString() == messageId) {
            MessageLogEntry *messageEntry = dynamic_cast<MessageLogEntry*>(entry);
            return messageEntry;
        }
    }

    return 0;
}

