/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "chatmanager.h"
#include "telepathyhelper.h"
#include "phoneutils.h"
#include "config.h"

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>

ChatManager::ChatManager(QObject *parent)
: QObject(parent)
{
    // wait one second for other acknowledge calls before acknowledging messages to avoid many round trips
    mMessagesAckTimer.setInterval(1000);
    mMessagesAckTimer.setSingleShot(true);
    connect(&mMessagesAckTimer, SIGNAL(timeout()), SLOT(onAckTimerTriggered()));
}

ChatManager *ChatManager::instance()
{
    static ChatManager *manager = new ChatManager();
    return manager;
}

void ChatManager::sendMessage(const QString &phoneNumber, const QString &message)
{
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->call("SendMessage", phoneNumber, message);
}

int ChatManager::unreadMessagesCount() const
{
    int count = 0;
    Q_FOREACH(const Tp::TextChannelPtr &channel, mChannels.values()) {
        count += channel->messageQueue().count();
    }

    return count;
}

int ChatManager::unreadMessages(const QString &phoneNumber)
{
    Tp::TextChannelPtr channel = existingChat(phoneNumber);
    if (channel.isNull()) {
        return 0;
    }

    return channel->messageQueue().count();
}

void ChatManager::onTextChannelAvailable(Tp::TextChannelPtr channel)
{
    QString id = channel->targetContact()->id();
    mChannels[id] = channel;

    connect(channel.data(),
            SIGNAL(messageReceived(Tp::ReceivedMessage)),
            SLOT(onMessageReceived(Tp::ReceivedMessage)));
    connect(channel.data(),
            SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
            SLOT(onMessageSent(Tp::Message,Tp::MessageSendingFlags,QString)));
    connect(channel.data(),
            SIGNAL(pendingMessageRemoved(const Tp::ReceivedMessage&)),
            SLOT(onPendingMessageRemoved(const Tp::ReceivedMessage&)));

    Q_EMIT unreadMessagesChanged(id);

    Q_FOREACH(const Tp::ReceivedMessage &message, channel->messageQueue()) {
        onMessageReceived(message);
    }
}

void ChatManager::onMessageReceived(const Tp::ReceivedMessage &message)
{
    // ignore delivery reports for now
    // FIXME: we need to handle errors on sending messages at some point
    if (message.isDeliveryReport()) {
        return;
    }

    Q_EMIT messageReceived(message.sender()->id(), message.text(), message.received(), message.messageToken(), true);
    Q_EMIT unreadMessagesChanged(message.sender()->id());
}

void ChatManager::onPendingMessageRemoved(const Tp::ReceivedMessage &message)
{
    // emit the signal saying the unread messages for a specific number has changed
    Q_EMIT unreadMessagesChanged(message.sender()->id());
}

void ChatManager::onMessageSent(const Tp::Message &sentMessage, const Tp::MessageSendingFlags flags, const QString &message)
{
    Q_UNUSED(message)
    Q_UNUSED(flags)

    Tp::TextChannel *channel = qobject_cast<Tp::TextChannel*>(sender());
    if (!channel) {
        return;
    }

    Q_EMIT messageSent(channel->targetContact()->id(), sentMessage.text());
}

Tp::TextChannelPtr ChatManager::existingChat(const QString &phoneNumber)
{
    Tp::TextChannelPtr channel;
    Q_FOREACH(const QString &key, mChannels.keys()) {
        if (PhoneUtils::isSameContact(key, phoneNumber)) {
            channel = mChannels[key];
            break;
        }
    }

    return channel;
}

void ChatManager::acknowledgeMessage(const QString &phoneNumber, const QString &messageId)
{
    mMessagesAckTimer.start();
    mMessagesToAck[phoneNumber].append(messageId);
}

void ChatManager::onAckTimerTriggered()
{
    // ack all pending messages
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();

    QMap<QString, QStringList>::const_iterator it = mMessagesToAck.constBegin();
    while (it != mMessagesToAck.constEnd()) {
        phoneAppHandler->call("AcknowledgeMessages", it.key(), it.value());
        ++it;
    }

    mMessagesToAck.clear();
}
