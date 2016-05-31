/*
 * Copyright (C) 2012-2015 Canonical, Ltd.
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

TextHandler::TextHandler(QObject *parent)
: QObject(parent)
{
    qDBusRegisterMetaType<AttachmentStruct>();
    qDBusRegisterMetaType<AttachmentList>();
    qRegisterMetaType<PendingMessage>();
}

TextHandler *TextHandler::instance()
{
    static TextHandler *handler = new TextHandler();
    return handler;
}

void TextHandler::startChat(const QString &accountId, const QVariantMap &properties)
{
    ChatStartingJob *job = new ChatStartingJob(this, accountId, properties);
    QTimer::singleShot(0, job, &ChatStartingJob::startJob);
    // FIXME: do we care about the result?
}

QString TextHandler::sendMessage(const QString &accountId, const QString &message, const AttachmentList &attachments, const QVariantMap &properties)
{
    PendingMessage pendingMessage = {accountId, message, attachments, properties};
    MessageSendingJob *job = new MessageSendingJob(this, pendingMessage);
    QTimer::singleShot(0, job, &MessageSendingJob::startJob);

    return job->objectPath();
}

void TextHandler::acknowledgeMessages(const QStringList &recipients, const QStringList &messageIds, const QString &accountId)
{
    QVariantMap properties;
    properties["participantIds"] = recipients;

    QList<Tp::TextChannelPtr> channels = existingChannels(accountId, properties);
    if (channels.isEmpty()) {
        return;
    }

    QList<Tp::ReceivedMessage> messagesToAck;
    Q_FOREACH(const Tp::TextChannelPtr &channel, channels) {
        Q_FOREACH(const Tp::ReceivedMessage &message, channel->messageQueue()) {
            if (messageIds.contains(message.messageToken())) {
                messagesToAck.append(message);
            }
        }
        channel->acknowledge(messagesToAck);
    }
}

void TextHandler::acknowledgeAllMessages(const QStringList &recipients, const QString &accountId)
{
    QVariantMap properties;
    properties["participantIds"] = recipients;
    QList<Tp::TextChannelPtr> channels = existingChannels(accountId, properties);
    if (channels.isEmpty()) {
        return;
    }

    Q_FOREACH(const Tp::TextChannelPtr &channel, channels) {
        channel->acknowledge(channel->messageQueue());
    }
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
    if (chatType == 0 && targetIds.size() == 1) {
        chatType = 1;
    }
    QString roomId = properties["threadId"].toString();

    Q_FOREACH(const Tp::TextChannelPtr &channel, mChannels) {
        int count = 0;
        AccountEntry *channelAccount = TelepathyHelper::instance()->accountForConnection(channel->connection());

        if (!channelAccount || channelAccount->accountId() != accountId) {
            continue;
        }

        if (chatType != channel->targetHandleType()) {
            continue;
        }

        if (chatType == 2) {
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
