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

#include "texthandler.h"
#include "phoneutils.h"
#include "telepathyhelper.h"
#include "config.h"
#include "dbustypes.h"
#include "accountentry.h"
#include "chatstartingjob.h"

#include <QImage>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>
#include <TelepathyQt/PendingChannelRequest>
#include <QDBusMessage>
#include <QDBusConnection>
#include <History/TextEvent>
#include <History/Manager>

TextHandler::TextHandler(QObject *parent)
: QObject(parent)
  , mMessagingAppMonitor("com.canonical.MessagingApp", QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForRegistration|QDBusServiceWatcher::WatchForUnregistration), mMessagingAppRegistered(false)
{
    qDBusRegisterMetaType<AttachmentStruct>();
    qDBusRegisterMetaType<AttachmentList>();
    qRegisterMetaType<PendingMessage>();
    connect(&mMessagingAppMonitor, SIGNAL(serviceRegistered(const QString&)), SLOT(onMessagingAppOpen()));
    connect(&mMessagingAppMonitor, SIGNAL(serviceUnregistered(const QString&)), SLOT(onMessagingAppClosed()));
    connect(TelepathyHelper::instance(), &TelepathyHelper::accountAdded, [=](AccountEntry *account) {
        if (mMessagingAppRegistered && !account->active() && account->protocolInfo()->leaveRoomsOnClose()) {
            account->reconnect();
        }
    });
}

void TextHandler::onMessagingAppOpen()
{
    mMessagingAppRegistered = true;
    Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->accounts()) {
        if (!account->active() && account->protocolInfo()->leaveRoomsOnClose()) {
            account->reconnect();
        }
    }
}

void TextHandler::onMessagingAppClosed()
{
    mMessagingAppRegistered = false;
    Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->accounts()) {
        if (account->protocolInfo()->leaveRoomsOnClose()) {
            account->requestDisconnect();
        }
    }

}

TextHandler *TextHandler::instance()
{
    static TextHandler *handler = new TextHandler();
    return handler;
}

QString TextHandler::startChat(const QString &accountId, const QVariantMap &properties)
{
    ChatStartingJob *job = new ChatStartingJob(this, accountId, properties);
    job->startJob();
    return job->objectPath();
}

QString TextHandler::sendMessage(const QString &accountId, const QString &message, const AttachmentList &attachments, const QVariantMap &properties)
{
    PendingMessage pendingMessage = {accountId, message, attachments, properties};
    MessageSendingJob *job = new MessageSendingJob(this, pendingMessage);
    job->startJob();

    return job->objectPath();
}

void TextHandler::acknowledgeMessages(const QVariantList &messages)
{
    Q_FOREACH(const QVariant &message, messages) {
        QVariantMap properties = qdbus_cast<QVariantMap>(message);
        QList<Tp::TextChannelPtr> channels = existingChannels(properties["accountId"].toString(), properties);
        if (channels.isEmpty()) {
            return;
        }

        QList<Tp::ReceivedMessage> messagesToAck;
        QString messageId = properties["messageId"].toString();
        Q_FOREACH(const Tp::TextChannelPtr &channel, channels) {
            Q_FOREACH(const Tp::ReceivedMessage &message, channel->messageQueue()) {
                if (messageId == message.messageToken()) {
                    messagesToAck.append(message);
                }
            }
            channel->acknowledge(messagesToAck);
        }
    }
}

void TextHandler::acknowledgeAllMessages(const QVariantMap &properties)
{
    QList<Tp::TextChannelPtr> channels = existingChannels(properties["accountId"].toString(), properties);
    if (channels.isEmpty()) {
        return;
    }

    Q_FOREACH(const Tp::TextChannelPtr &channel, channels) {
        channel->acknowledge(channel->messageQueue());
    }
}

void TextHandler::redownloadMessage(const QString &accountId, const QString &threadId, const QString &eventId)
{

    History::TextEvent textEvent = History::Manager::instance()->getSingleEvent(History::EventTypeText, accountId, threadId, eventId);
    if (textEvent.isNull()) {
      qWarning() << "No message for re-download found under accountId: " << accountId << ", threadId: " << threadId << ", eventId: " << eventId;
      return;
    }


    // Only re-download temporarily failed messages.
    if (textEvent.messageStatus() != History::MessageStatusTemporarilyFailed) {
      qWarning() << "Trying to re-download message with wrong status: " << textEvent.messageStatus();
      return;
    }

    //Update status to pending
    textEvent.setMessageStatus(History::MessageStatusPending);
    History::Events events;
    events.append(textEvent);
    if (!History::Manager::instance()->writeEvents(events)) {
      qWarning() << "Failed to save the re-downloaded message pending status!";
    }

    QDBusMessage request;
    request = QDBusMessage::createMethodCall("org.ofono.mms",
                                   eventId, "org.ofono.mms.Message",
                                   "Redownload");
    QDBusConnection::sessionBus().call(request);
}

bool TextHandler::destroyTextChannel(const QString &objectPath)
{
    Tp::TextChannelPtr channelToDestroy = existingChannelFromObjectPath(objectPath);
    if (!channelToDestroy ||
        !channelToDestroy->hasInterface(TP_QT_IFACE_CHANNEL_INTERFACE_DESTROYABLE)) {
        return false;
    }

    Tp::Client::ChannelInterfaceDestroyableInterface *interface = channelToDestroy->interface<Tp::Client::ChannelInterfaceDestroyableInterface>();
    if (!interface) {
        return false;
    }

    // FIXME(boiko): maybe use an async API? not sure it is worth adding that
    QDBusPendingReply<void> reply = interface->Destroy();
    reply.waitForFinished();
    return !reply.isError();
}

bool TextHandler::changeRoomTitle(const QString &objectPath, const QString &title)
{
    qDebug() << __PRETTY_FUNCTION__;
    Tp::TextChannelPtr channel = existingChannelFromObjectPath(objectPath);
    if (!channel) {
        qWarning() << "Could not find channel for object path" << objectPath;
        return false;
    }

    Tp::Client::ChannelInterfaceRoomConfigInterface *roomConfigInterface;
    roomConfigInterface = channel->optionalInterface<Tp::Client::ChannelInterfaceRoomConfigInterface>();
    if (!roomConfigInterface) {
        qWarning() << "Could not find RoomConfig interface in the channel" << objectPath;
        return false;
    }

    QVariantMap properties;
    properties["Title"] = title;
    QDBusPendingReply<void> reply = roomConfigInterface->UpdateConfiguration(properties);
    reply.waitForFinished();
    return !reply.isError();
}

void TextHandler::onTextChannelInvalidated()
{
    Tp::TextChannelPtr textChannel(qobject_cast<Tp::TextChannel*>(sender()));
    mChannels.removeAll(textChannel);
}

void TextHandler::onTextChannelAvailable(Tp::TextChannelPtr channel)
{
    qDebug() << "TextHandler::onTextChannelAvailable" << channel;
    AccountEntry *account = TelepathyHelper::instance()->accountForConnection(channel->connection());
    if (!account) {
        return;
    }
    connect(channel.data(),
            SIGNAL(invalidated(Tp::DBusProxy*,const QString&, const QString&)),
            SLOT(onTextChannelInvalidated()));

    QString accountId = account->accountId();
    mChannels.append(channel);
}

QList<Tp::TextChannelPtr> TextHandler::existingChannels(const QString &accountId, const QVariantMap &properties)
{
    QList<Tp::TextChannelPtr> channels;
    QStringList targetIds = properties["participantIds"].toStringList();
    int chatType = properties["chatType"].toUInt();
    if (chatType == Tp::HandleTypeNone && targetIds.size() == 1) {
        chatType = Tp::HandleTypeContact;
    }

    QString roomId = properties["threadId"].toString();

    // try to use the threadId as participantId if empty
    if (chatType == Tp::HandleTypeContact && targetIds.isEmpty()) {
        targetIds << roomId;
    }

    Q_FOREACH(const Tp::TextChannelPtr &channel, mChannels) {
        int count = 0;
        AccountEntry *channelAccount = TelepathyHelper::instance()->accountForConnection(channel->connection());

        if (!channelAccount || channelAccount->accountId() != accountId) {
            continue;
        }

        if (chatType != channel->targetHandleType()) {
            continue;
        }

        if (chatType == Tp::HandleTypeRoom) {
            if (!roomId.isEmpty() && channel->targetHandleType() == chatType && roomId == channel->targetId()) {
                channels.append(channel);
            }
            continue;
        }

        // this is a special case. We have to check if we are looking for a channel open with our self contact.
        bool channelToSelfContact = channel->groupContacts(true).size() == 1 && targetIds.size() == 1 &&
                          channel->targetHandleType() == Tp::HandleTypeContact &&
                          channelAccount->compareIds(channel->targetId(), channelAccount->selfContactId()) &&
                          channelAccount->compareIds(targetIds.first(), channel->targetId()); // make sure we compare the recipient with the channel

        if (channelToSelfContact) {
            channels.append(channel);
            continue;
        }

        if (channel->groupContacts(false).size() != targetIds.size()) {
            continue;
        }

        Q_FOREACH(const QString &targetId, targetIds) {
            Q_FOREACH(const Tp::ContactPtr &channelContact, channel->groupContacts(false)) {
                if (channelAccount->compareIds(channelContact->id(), targetId)) {
                    count++;
                }
            }
        }
        if (count == targetIds.size()) {
            channels.append(channel);
        }
    }
    return channels;
}

Tp::TextChannelPtr TextHandler::existingChannelFromObjectPath(const QString &objectPath)
{
    Q_FOREACH(Tp::TextChannelPtr channel, mChannels) {
        if (channel->objectPath() == objectPath) {
            return channel;
        }
    }
    return Tp::TextChannelPtr();
}

void TextHandler::inviteParticipants(const QString &objectPath, const QStringList &participants, const QString &message)
{
    Tp::TextChannelPtr channel = existingChannelFromObjectPath(objectPath);
    if (channel->targetHandleType() != Tp::HandleTypeRoom || !channel) {
        return;
    }
    if (!channel->groupCanAddContacts() || !channel->connection()) {
        return;
    }
    Tp::PendingContacts *contactOp = channel->connection()->contactManager()->contactsForIdentifiers(participants);
    connect(contactOp, &Tp::PendingOperation::finished, [=] {
        if (contactOp->isError()) {
            return;
        }
        channel->groupAddContacts(contactOp->contacts(), message);
    });

}

void TextHandler::removeParticipants(const QString &objectPath, const QStringList &participants, const QString &message)
{
    Tp::TextChannelPtr channel = existingChannelFromObjectPath(objectPath);
    if (channel->targetHandleType() != Tp::HandleTypeRoom || !channel) {
        return;
    }
    if (!channel->groupCanRemoveContacts() || !channel->connection()) {
        return;
    }
    Tp::PendingContacts *contactOp = channel->connection()->contactManager()->contactsForIdentifiers(participants);
    connect(contactOp, &Tp::PendingOperation::finished, [=] {
        if (contactOp->isError()) {
            return;
        }
        channel->groupRemoveContacts(contactOp->contacts(), message);
    });
}

bool TextHandler::leaveChat(const QString &objectPath, const QString &message)
{
    Tp::TextChannelPtr channel = existingChannelFromObjectPath(objectPath);
    if (channel->targetHandleType() != Tp::HandleTypeRoom || !channel || !channel->connection()) {
        return false;
    }
    channel->requestLeave(message);
    return true;
}

void TextHandler::leaveRooms(const QString &accountId, const QString &message)
{
    Q_FOREACH(const Tp::TextChannelPtr &channel, mChannels) {
        if (channel->targetHandleType() != Tp::HandleTypeRoom) {
            continue;
        }
        AccountEntry *account = TelepathyHelper::instance()->accountForConnection(channel->connection());
        if (account && account->accountId() == accountId) {
            leaveChat(channel->objectPath(), message);
        }
    }
}
