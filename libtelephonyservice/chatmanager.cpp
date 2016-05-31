/*
 * Copyright (C) 2012-2016 Canonical, Ltd.
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
    mMessagesAckTimer.setInterval(25);
    mMessagesAckTimer.setSingleShot(true);
    connect(TelepathyHelper::instance(), SIGNAL(channelObserverUnregistered()), SLOT(onChannelObserverUnregistered()));
    connect(&mMessagesAckTimer, SIGNAL(timeout()), SLOT(onAckTimerTriggered()));
    connect(TelepathyHelper::instance(), SIGNAL(setupReady()), SLOT(onConnectedChanged()));
}

void ChatManager::onChannelObserverUnregistered()
{
    mTextChannels.clear();
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

QString ChatManager::sendMessage(const QString &accountId, const QString &message, const QVariant &attachments, const QVariantMap &properties)
{
    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);

    if (!account) {
        return QString();
    }

    QVariantMap propMap = properties;

    // check if files should be copied to a temporary location before passing them to handler
    bool tmpFiles = (properties.contains("x-canonical-tmp-files") && properties["x-canonical-tmp-files"].toBool());

    // participants coming from qml are variants
    if (properties.contains("participantIds")) {
        propMap["participantIds"] = properties["participantIds"].toStringList();
    }

    AttachmentList newAttachments;
    Q_FOREACH (const QVariant &attachment, attachments.toList()) {
        AttachmentStruct newAttachment;
        QVariantList list = attachment.toList();
        newAttachment.id = list.at(0).toString();
        newAttachment.contentType = list.at(1).toString();

        if (tmpFiles) {
            // we can't give the original path to handler, as it might be removed
            // from history database by the time it tries to read the file,
            // so we duplicate the file and the handler will remove it
            QTemporaryFile tmpFile("/tmp/XXXXX");
            tmpFile.setAutoRemove(false);
            if (!tmpFile.open()) {
                qWarning() << "Unable to create a temporary file";
                return QString();
            }
            QFile originalFile(list.at(2).toString());
            if (!originalFile.open(QIODevice::ReadOnly)) {
                qWarning() << "Attachment file not found";
                return QString();
            }
            if (tmpFile.write(originalFile.readAll()) == -1) {
                qWarning() << "Failed to write attachment to a temporary file";
                return QString();
            }
            newAttachment.filePath = tmpFile.fileName();
            tmpFile.close();
            originalFile.close();
        } else {
            newAttachment.filePath = list.at(2).toString();
        }
        newAttachments << newAttachment;
    }

    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    QDBusReply<QString> reply = phoneAppHandler->call("SendMessage", account->accountId(), message, QVariant::fromValue(newAttachments), propMap);
    if (reply.isValid()) {
        return reply.value();
    }
    return QString();
}

QList<Tp::TextChannelPtr> ChatManager::channelForProperties(const QVariantMap &properties)
{
    // FIXME: implement
    /*
    QVariantMap propMap = properties;
    int chatType = 0;

    QStringList participants;
    // participants coming from qml are variants
    if (properties.contains("participantIds")) {
        participants = properties["participantIds"].toStringList();
        if (!participants.isEmpty()) {
            propMap["participantIds"] = participants;
        }
    }

    if (participants.isEmpty() && propMap.contains("participants")) {
        // try to generate list of participants from "participants"
        Q_FOREACH(const QVariant &participantMap, propMap["participants"].toList()) {
            if (participantMap.toMap().contains("identifier")) {
                participants << participantMap.toMap()["identifier"].toString();
            }
        }
        if (!participants.isEmpty()) {
            propMap["participantIds"] = participants;
        }
    }

    if (properties.contains("chatType")) {
        chatType = properties["chatType"].toInt();
    } else {
        if (participants.length() == 1) {
            chatType = 1;
        }
    }

    if ((participants.count() == 0 && chatType == 1)  || accountId.isEmpty()) {
        return NULL;
    }

    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);

    if (!account) {
        return NULL;
    }

    Q_FOREACH (ChatEntry *chatEntry, mChatEntries) {
        int participantCount = 0;

        if (chatType == 2) {
            QString roomId = propMap["threadId"].toString();
            if (!roomId.isEmpty() && chatEntry->chatType() == 2 && roomId == chatEntry->chatId()) {
                return chatEntry;
            }
            continue;
        }

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
        phoneAppHandler->call("StartChat", accountId, propMap);
    }
    return NULL;*/
}

Tp::TextChannelPtr ChatManager::channelForObjectPath(const QString &objectPath)
{
    Q_FOREACH(Tp::TextChannelPtr channel, mTextChannels) {
        if (channel->objectPath() == objectPath) {
            return channel;
        }
    }
    return Tp::TextChannelPtr();
}

void ChatManager::onTextChannelAvailable(Tp::TextChannelPtr channel)
{
    mTextChannels << channel;
    connect(channel.data(),
            SIGNAL(invalidated(Tp::DBusProxy*,const QString&, const QString&)),
            SLOT(onChannelInvalidated()));

    Q_EMIT textChannelAvailable(channel);
}

void ChatManager::onChannelInvalidated()
{
    Tp::TextChannelPtr channel(qobject_cast<Tp::TextChannel*>(sender()));
    mTextChannels.removeAll(channel);
    Q_EMIT textChannelInvalidated(channel);
}

void ChatManager::acknowledgeMessage(const QStringList &recipients, const QString &messageId, const QString &accountId)
{
    // FIXME: this is broken for chat rooms and needs a better approach
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
            phoneAppHandler->asyncCall("AcknowledgeMessages", it2.key(), it2.value(), accountId);
            ++it2;
        }
        ++it;
    }

    mMessagesToAck.clear();
}
