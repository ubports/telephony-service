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
: QObject(parent),
  mReady(TelepathyHelper::instance()->ready())
{
    qDBusRegisterMetaType<AttachmentList>();
    qDBusRegisterMetaType<AttachmentStruct>();
    // wait one second for other acknowledge calls before acknowledging messages to avoid many round trips
    mMessagesAckTimer.setInterval(1000);
    mMessagesAckTimer.setSingleShot(true);
    connect(TelepathyHelper::instance(), SIGNAL(channelObserverUnregistered()), SLOT(onChannelObserverUnregistered()));
    connect(TelepathyHelper::instance(), SIGNAL(setupReady()), SLOT(onTelepathyReady()));
    connect(&mMessagesAckTimer, SIGNAL(timeout()), SLOT(onAckTimerTriggered()));
    connect(TelepathyHelper::instance(), SIGNAL(setupReady()), SLOT(onConnectedChanged()));
}

void ChatManager::onTelepathyReady()
{
    mReady = true;
    Q_FOREACH(const Tp::TextChannelPtr &channel, mPendingChannels) {
        onTextChannelAvailable(channel);
    }
    mPendingChannels.clear();
}

void ChatManager::onChannelObserverUnregistered()
{
    QList<ChatEntry*> tmp = mChatEntries;
    mChatEntries.clear();
    Q_EMIT chatsChanged();
    Q_FOREACH(ChatEntry *entry, tmp) {
        // for some reason deleteLater is not working
        delete entry;
    }
}

void ChatManager::onConnectedChanged()
{
    if (TelepathyHelper::instance()->ready()) {
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
    if (!mReady) {
        mPendingChannels.append(channel);
        return;
    }
    ChatEntry *chatEntry = new ChatEntry(channel, this);
    mChatEntries.append(chatEntry);

    connect(channel.data(),
            SIGNAL(messageReceived(Tp::ReceivedMessage)),
            SLOT(onMessageReceived(Tp::ReceivedMessage)));
    connect(channel.data(),
            SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
            SLOT(onMessageSent(Tp::Message,Tp::MessageSendingFlags,QString)));
     connect(channel.data(),
            SIGNAL(invalidated(Tp::DBusProxy*,const QString&, const QString&)),
            SLOT(onChannelInvalidated()));

    Q_FOREACH(const Tp::ReceivedMessage &message, channel->messageQueue()) {
        onMessageReceived(message);
    }

    Q_EMIT chatsChanged();
    Q_EMIT chatEntryCreated(chatEntry->account()->accountId(), chatEntry->participants(), chatEntry);
}

void ChatManager::onChannelInvalidated()
{
    Tp::TextChannelPtr channel(qobject_cast<Tp::TextChannel*>(sender()));
    ChatEntry *chatEntry = chatEntryForChannel(channel);
    if (chatEntry) {
        mChatEntries.removeAll(chatEntry);
        // for some reason deleteLater is not working
        delete chatEntry;
        Q_EMIT chatsChanged();
    }
}

ChatEntry *ChatManager::chatEntryForChannel(const Tp::TextChannelPtr &channel)
{
    Q_FOREACH (ChatEntry *chatEntry, mChatEntries) {
        if (channel == chatEntry->channel()) {
            return chatEntry;
        }
    }
    return NULL;
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
    phoneAppHandler->asyncCall("AcknowledgeAllMessages", recipients, accountId);
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

QList<ChatEntry*> ChatManager::chatEntries() const
{
    return mChatEntries;
}

ChatEntry *ChatManager::chatEntryForParticipants(const QString &accountId, const QStringList &participants, bool create)
{
    if (participants.count() == 0 || accountId.isEmpty()) {
        return NULL;
    }

    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);

    if (!account) {
        return NULL;
    }

    Q_FOREACH (ChatEntry *chatEntry, mChatEntries) {
        int participantCount = 0;
        Tp::Contacts contacts = chatEntry->channel()->groupContacts(false);
        if (participants.count() != contacts.count()) {
            continue;
        }
        // iterate over participants
        Q_FOREACH (const Tp::ContactPtr &contact, contacts) {
            if (account->type() == AccountEntry::PhoneAccount || account->type() == AccountEntry::MultimediaAccount) {
                Q_FOREACH(const QString &participant, participants) {
                    if (PhoneUtils::comparePhoneNumbers(participant, contact->id()) > PhoneUtils::NO_MATCH) {
                        participantCount++;
                        break;
                    }
                }
                continue;
            }
            if (participants.contains(contact->id())) {
                participantCount++;
            } else {
                break;
            }
        }
        if (participantCount == participants.count()) {
            return chatEntry;
        }
    }

    if (create) {
        QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
        phoneAppHandler->call("StartChat", accountId, participants);
    }
    return NULL;
}

ChatEntry *ChatManager::chatEntryForChatRoom(const QString &accountId, const QVariantMap &properties, bool create)
{
    Q_UNUSED(accountId)
    Q_UNUSED(properties)
    Q_UNUSED(create)
    // FIXME: implement
}

QQmlListProperty<ChatEntry> ChatManager::chats()
{
    return QQmlListProperty<ChatEntry>(this, 0, chatCount, chatAt);
}

int ChatManager::chatCount(QQmlListProperty<ChatEntry> *p)
{
    return ChatManager::instance()->chatEntries().count();
}

ChatEntry *ChatManager::chatAt(QQmlListProperty<ChatEntry> *p, int index)
{
    return ChatManager::instance()->chatEntries()[index];
}

