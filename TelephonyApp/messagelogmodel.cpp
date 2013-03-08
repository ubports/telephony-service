/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
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

MessageLogModel::MessageLogModel(QObject *parent) :
    ConversationFeedModel(parent)
{
}

void MessageLogModel::appendMessage(const QString &number,
                                    const QString &message,
                                    bool incoming,
                                    const QDateTime &timestamp,
                                    const QString &messageId,
                                    bool unread)
{
    // check if we already have the given message
    if (messageById(messageId) != 0) {
        return;
    }

    MessageLogEntry *entry = new MessageLogEntry(this);
    entry->setIncoming(incoming);
    entry->setPhoneNumber(number);
    entry->setMessage(message);
    entry->setTimestamp(timestamp);
    entry->setMessageId(messageId);
    // set the alias to the phone number as a fallback in case the contact is not known
    entry->setContactAlias(number);

    // if the item is incoming, mark it as new
    entry->setNewItem(unread);

    connect(entry,
            SIGNAL(newItemChanged()),
            SLOT(onNewItemChanged()));

    ContactEntry *contact = ContactModel::instance()->contactFromPhoneNumber(number);
    if (contact) {
        fillContactInfo(entry, contact);
    } else {
        checkNonStandardNumbers(entry);
    }
    addItem(entry);
}

void MessageLogModel::onMessageReceived(const QString &number,
                                        const QString &message,
                                        const QDateTime &timestamp,
                                        const QString &messageId, bool unread)
{
    // check if the message is already in the model (it might have been loaded from logger)
    if (!messageId.isEmpty() && messageById(messageId) != 0) {
        return;
    }
    appendMessage(number, message, true, timestamp, messageId, unread);
}

void MessageLogModel::onMessageSent(const QString &number, const QString &message)
{
    appendMessage(number, message, false);
}

void MessageLogModel::onNewItemChanged()
{
    MessageLogEntry *entry = qobject_cast<MessageLogEntry*>(sender());
    if (!entry) {
        return;
    }

    Q_EMIT messageRead(entry->phoneNumber(), entry->messageId());
}

MessageLogEntry *MessageLogModel::messageById(const QString &messageId)
{

    Q_FOREACH(ConversationFeedItem *entry, mItems) {
        MessageLogEntry *messageEntry = dynamic_cast<MessageLogEntry*>(entry);
        if (messageEntry && messageEntry->messageId() == messageId) {
            return messageEntry;
        }
    }

    return 0;
}

bool MessageLogModel::matchesSearch(const QString &searchTerm, const QModelIndex &index) const
{
    bool foundMatch = false;
    MessageLogEntry *entry = dynamic_cast<MessageLogEntry*>(entryFromIndex(index));
    if (!entry) {
        return false;
    }

    // Test the message text. Even if onlyLatest is set, we return all text entries that match
    QString value = entry->message();
    if (value.indexOf(searchTerm, 0, Qt::CaseInsensitive) >= 0) {
        foundMatch = true;
    }

    return foundMatch || ConversationFeedModel::matchesSearch(searchTerm, index);
}

QString MessageLogModel::itemType(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return "message";
}
