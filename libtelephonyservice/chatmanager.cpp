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
#include "accountentry.h"

#include <TelepathyQt/Contact>
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
    connect(TelepathyHelper::instance(), SIGNAL(connectedChanged()), SLOT(onConnectedChanged()));
}

void ChatManager::onConnectedChanged()
{
    if (TelepathyHelper::instance()->connected()) {
        onAckTimerTriggered();
    }
}

ChatManager *ChatManager::instance()
{
    static ChatManager *manager = new ChatManager();
    return manager;
}

void ChatManager::sendMMS(const QStringList &recipients, const QString &message, const QVariant &attachments, const QString &accountId)
{
    AttachmentList newAttachments;
    AccountEntry *account = NULL;
    if (accountId.isNull() || accountId.isEmpty()) {
        account = TelepathyHelper::instance()->defaultMessagingAccount();
        if (!account && !TelepathyHelper::instance()->activeAccounts().isEmpty()) {
            account = TelepathyHelper::instance()->activeAccounts()[0];
        }
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
        textAttachment.id = "text_0.txt";
        textAttachment.contentType = "text/plain;charset=UTF-8";
        textAttachment.filePath = textFile.fileName();
        newAttachments << textAttachment;
    }

    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->call("SendMMS", recipients, QVariant::fromValue(newAttachments), account->accountId());
}

void ChatManager::sendMessage(const QStringList &recipients, const QString &message, const QString &accountId)
{
    // FIXME: this probably should be handle internally by telepathy-ofono
    if (recipients.size() > 1 && TelepathyHelper::instance()->mmsGroupChat()) {
        sendMMS(recipients, message, QVariant(), accountId);
        return;
    }
    AccountEntry *account = NULL;
    if (accountId.isNull() || accountId.isEmpty()) {
        account = TelepathyHelper::instance()->defaultMessagingAccount();
        if (!account && !TelepathyHelper::instance()->activeAccounts().isEmpty()) {
            account = TelepathyHelper::instance()->activeAccounts()[0];
        }
    } else {
        account = TelepathyHelper::instance()->accountForId(accountId);
    }

    if (!account) {
        return;
    }

    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->call("SendMessage", recipients, message, account->accountId());
}

void ChatManager::onTextChannelAvailable(Tp::TextChannelPtr channel)
{
    mChannels.append(channel);

    connect(channel.data(),
            SIGNAL(messageReceived(Tp::ReceivedMessage)),
            SLOT(onMessageReceived(Tp::ReceivedMessage)));
    connect(channel.data(),
            SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
            SLOT(onMessageSent(Tp::Message,Tp::MessageSendingFlags,QString)));

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
}

void ChatManager::onMessageSent(const Tp::Message &sentMessage, const Tp::MessageSendingFlags flags, const QString &message)
{
    Q_UNUSED(message)
    Q_UNUSED(flags)

    Tp::TextChannel *channel = qobject_cast<Tp::TextChannel*>(sender());
    if (!channel) {
        return;
    }

    QStringList recipients;
    Q_FOREACH(const Tp::ContactPtr &contact, channel->groupContacts(false)) {
        recipients << contact->id();
    }

    Q_EMIT messageSent(recipients, sentMessage.text());
}

void ChatManager::acknowledgeMessage(const QStringList &recipients, const QString &messageId, const QString &accountId)
{
    AccountEntry *account = NULL;
    if (accountId.isNull() || accountId.isEmpty()) {
        account = TelepathyHelper::instance()->defaultMessagingAccount();
        if (!account && !TelepathyHelper::instance()->activeAccounts().isEmpty()) {
            account = TelepathyHelper::instance()->activeAccounts()[0];
        }
    } else {
        account = TelepathyHelper::instance()->accountForId(accountId);
    }

    if (!account) {
        mMessagesToAck[accountId][recipients].append(messageId);
        return;
    }

    mMessagesAckTimer.start();
    mMessagesToAck[account->accountId()][recipients].append(messageId);
}

void ChatManager::acknowledgeAllMessages(const QStringList &recipients, const QString &accountId)
{
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->call("AcknowledgeAllMessages", recipients, accountId);
}

void ChatManager::onAckTimerTriggered()
{
    // ack all pending messages
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();

    QMap<QString, QMap<QStringList,QStringList> >::const_iterator it = mMessagesToAck.constBegin();
    while (it != mMessagesToAck.constEnd()) {
        QString accountId = it.key();
        QMap<QStringList, QStringList>::const_iterator it2 = it.value().constBegin();
        while (it2 != it.value().constEnd()) {
            phoneAppHandler->call("AcknowledgeMessages", it2.key(), it2.value(), accountId);
            ++it2;
        }
        ++it;
    }

    mMessagesToAck.clear();
}
