/*
 * Copyright (C) 2012-2016 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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
#include "chatentry.h"
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

QVariantMap convertPropertiesForDBus(const QVariantMap &properties)
{
    QVariantMap propMap = properties;
    // participants coming from qml are variants
    if (properties.contains("participantIds")) {
        QStringList participants = properties["participantIds"].toStringList();
        if (!participants.isEmpty()) {
            propMap["participantIds"] = participants;
        }
    }
    return propMap;
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

QString ChatManager::startChat(const QString &accountId, const QVariantMap &properties)
{
    QVariantMap propMap = convertPropertiesForDBus(properties);
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    QDBusReply<QString> reply = phoneAppHandler->call("StartChat", accountId, propMap);
    return reply.value();
}

QString ChatManager::sendMessage(const QString &accountId, const QString &message, const QVariant &attachments, const QVariantMap &properties)
{
    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);

    if (!account) {
        return QString();
    }

    QVariantMap propMap = convertPropertiesForDBus(properties);

    // check if files should be copied to a temporary location before passing them to handler
    bool tmpFiles = (properties.contains("x-canonical-tmp-files") && properties["x-canonical-tmp-files"].toBool());

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
    QList<Tp::TextChannelPtr> channels;


    Q_FOREACH (Tp::TextChannelPtr channel, mTextChannels) {
        if (channelMatchProperties(channel, properties)) {
            channels << channel;
        }
    }

    return channels;
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

bool ChatManager::channelMatchProperties(const Tp::TextChannelPtr &channel, const QVariantMap &properties)
{
    QVariantMap propMap = properties;
    ChatEntry::ChatType chatType = ChatEntry::ChatTypeNone;

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
        chatType = (ChatEntry::ChatType)properties["chatType"].toInt();
    } else {
        if (participants.length() == 1) {
            chatType = ChatEntry::ChatTypeContact;
        }
    }

    QString accountId;
    if (propMap.contains("accountId")) {
        accountId = propMap["accountId"].toString();
    }

    if (participants.count() == 0 && chatType == ChatEntry::ChatTypeContact) {
        return false;
    }

    AccountEntry *account = TelepathyHelper::instance()->accountForConnection(channel->connection());
    if (!account) {
        return false;
    }

    // only channels of the correct type should be returned
    if ((ChatEntry::ChatType)channel->targetHandleType() != chatType) {
        return false;
    }

    if (chatType == ChatEntry::ChatTypeRoom) {
        QString chatId = propMap["threadId"].toString();
        if (!chatId.isEmpty() && channel->targetId() == chatId) {
            // if we are filtering by one specific accountId, make sure it matches
            if (!accountId.isEmpty() && accountId != account->accountId()) {
                return false;
            }

            return true;
        }
        return false;
    }

    Tp::Contacts contacts = channel->groupContacts(false);
    if (participants.count() != contacts.count()) {
        return false;
    }
    int participantCount = 0;
    // iterate over participants
    Q_FOREACH (const Tp::ContactPtr &contact, contacts) {
        // try the easiest first
        if (participants.contains(contact->id())) {
            participantCount++;
            continue;
        }

        // if no exact match, try to use the account's compare function
        Q_FOREACH(const QString &participant, participants) {
            if (account->compareIds(participant, contact->id())) {
                participantCount++;
                break;
            }
        }
    }
    return (participantCount == participants.count());
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

void ChatManager::acknowledgeMessage(const QVariantMap &properties)
{
    mMessagesToAck << QVariant::fromValue(convertPropertiesForDBus(properties));
    mMessagesAckTimer.start();
}

void ChatManager::acknowledgeAllMessages(const QVariantMap &properties)
{
    qDebug() << "jezek - ChatManager::acknowledgeAllMessages";
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->asyncCall("AcknowledgeAllMessages", convertPropertiesForDBus(properties));
}

/**
 * Sends an asynchronous call through DBus to re-download a message identified by properties.
 * Parameters accountId, threadId, eventId uniquely identify the message, which shall be redownloaded.
 */
void ChatManager::redownloadMessage(const QString &accountId, const QString &threadId, const QString &eventId)
{
    qDebug() << "jezek - ChatManager::redownloadMessage";
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->asyncCall("RedownloadMessage", accountId, threadId, eventId);
}

void ChatManager::onAckTimerTriggered()
{
    // ack all pending messages
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();

    phoneAppHandler->asyncCall("AcknowledgeMessages", mMessagesToAck);

    mMessagesToAck.clear();
}

void ChatManager::leaveRooms(const QString &accountId, const QString &message)
{
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->asyncCall("LeaveRooms", accountId, message);
}

void ChatManager::leaveRoom(const QVariantMap &properties, const QString &message)
{
    QList<Tp::TextChannelPtr> channels = channelForProperties(properties);
    if (channels.isEmpty()) {
        return;
    }
    QDBusInterface *handlerIface = TelepathyHelper::instance()->handlerInterface();
    handlerIface->asyncCall("LeaveChat", channels.first()->objectPath(), message);
}
