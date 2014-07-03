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
#include "dbustypes.h"

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>
#include <QDBusArgument>

QDBusArgument &operator<<(QDBusArgument &argument, const AttachmentStruct &attachment)
{
    argument.beginStructure();
    argument << attachment.id << attachment.contentType << attachment.filePath;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, AttachmentStruct &attachment)
{
    argument.beginStructure();
    argument >> attachment.id >> attachment.contentType >> attachment.filePath;
    argument.endStructure();
    return argument;
}
ChatManager::ChatManager(QObject *parent)
: QObject(parent)
{
    qDBusRegisterMetaType<AttachmentList>();
    qDBusRegisterMetaType<AttachmentStruct>();
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

void ChatManager::sendMMS(const QStringList &phoneNumbers, const QString &message, const QVariant &attachments, const QString &accountId)
{
    AttachmentList newAttachments;
    Tp::AccountPtr account;
    if (accountId.isNull()) {
        account = TelepathyHelper::instance()->accounts()[0];
    } else {
        account = TelepathyHelper::instance()->accountForId(accountId);
    }

    Q_FOREACH (const QVariant &attachment, attachments.toList()) {
        AttachmentStruct newAttachment;
        QVariantList list = attachment.toList();
        newAttachment.id = list.at(0).toString();
        newAttachment.contentType = list.at(1).toString();
        newAttachment.filePath = list.at(2).toString();
        newAttachments << newAttachment;
    }

    if (!message.isEmpty()) {
        AttachmentStruct textAttachment;
        QTemporaryFile textFile("/tmp/XXXXX");
        textFile.setAutoRemove(false);
        if (!textFile.open()) {
            // FIXME: return error
            return;
        }
        textFile.write(message.toUtf8().data());
        textFile.close();
        textAttachment.id = "text";
        textAttachment.contentType = "text/plain;charset=UTF-8";
        textAttachment.filePath = textFile.fileName();
        newAttachments << textAttachment;
    }

    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->call("SendMMS", phoneNumbers, QVariant::fromValue(newAttachments), account->uniqueIdentifier());
}

void ChatManager::sendMessage(const QStringList &phoneNumbers, const QString &message, const QString &accountId)
{
    Tp::AccountPtr account;
    if (accountId.isNull()) {
        account = TelepathyHelper::instance()->accounts()[0];
    } else {
        account = TelepathyHelper::instance()->accountForId(accountId);
    }

    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->call("SendMessage", phoneNumbers, message, account->uniqueIdentifier());
}

void ChatManager::onTextChannelAvailable(Tp::TextChannelPtr channel)
{
    QString id = channel->targetContact()->id();
    mChannels.append(channel);

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

Tp::TextChannelPtr ChatManager::existingChat(const QStringList &phoneNumbers, const QString &accountId)
{
    Tp::TextChannelPtr channel;

    Q_FOREACH(const Tp::TextChannelPtr &channel, mChannels) {
        Tp::AccountPtr channelAccount = TelepathyHelper::instance()->accountForConnection(channel->connection());
        int count = 0;
        if (!channelAccount || channelAccount->uniqueIdentifier() != accountId
                || channel->groupContacts(false).size() != phoneNumbers.size()) {
            continue;
        }
        Q_FOREACH(const QString &phoneNumberNew, phoneNumbers) {
            Q_FOREACH(const Tp::ContactPtr &phoneNumberOld, channel->groupContacts(false)) {
                if (PhoneUtils::comparePhoneNumbers(phoneNumberOld->id(), phoneNumberNew)) {
                    count++;
                }
            }
        }
        if (count == phoneNumbers.size()) {
            return channel;
        }

    }

    return channel;
}

void ChatManager::acknowledgeMessage(const QString &phoneNumber, const QString &messageId, const QString &accountId)
{
    Tp::AccountPtr account;
    if (accountId.isNull()) {
        account = TelepathyHelper::instance()->accounts()[0];
    } else {
        account = TelepathyHelper::instance()->accountForId(accountId);
    }

    mMessagesAckTimer.start();
    mMessagesToAck[account->uniqueIdentifier()][phoneNumber].append(messageId);
}

void ChatManager::onAckTimerTriggered()
{
    // ack all pending messages
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();

    QMap<QString, QMap<QString,QStringList> >::const_iterator it = mMessagesToAck.constBegin();
    while (it != mMessagesToAck.constEnd()) {
        QString accountId = it.key();
        QMap<QString, QStringList>::const_iterator it2 = it.value().constBegin();
        while (it2 != it.value().constEnd()) {
            phoneAppHandler->call("AcknowledgeMessages", QStringList() << it2.key(), it2.value(), accountId);
            ++it2;
        }
        ++it;
    }

    mMessagesToAck.clear();
}
