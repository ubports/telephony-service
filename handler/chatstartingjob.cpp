/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "chatstartingjob.h"
#include "chatstartingjobadaptor.h"
#include "telepathyhelper.h"
#include "texthandler.h"
#include <TelepathyQt/PendingChannelRequest>

ChatStartingJob::ChatStartingJob(TextHandler *textHandler, const QString &accountId, const QVariantMap &properties)
: MessageJob(new ChatStartingJobAdaptor(this), textHandler), mTextHandler(textHandler), mAccountId(accountId), mProperties(properties)
{
    qDebug() << __PRETTY_FUNCTION__;
    connect(this, &ChatStartingJob::textChannelChanged, &ChatStartingJob::channelObjectPathChanged);
}

QString ChatStartingJob::accountId()
{
    return mAccountId;
}

void ChatStartingJob::startJob()
{
    qDebug() << __PRETTY_FUNCTION__;
    setStatus(Running);

    // Request the contact to start chatting to
    // FIXME: make it possible to select which account to use, for now, pick the first one
    AccountEntry *account = TelepathyHelper::instance()->accountForId(mAccountId);
    if (!account || !account->connected()) {
        qCritical() << "The selected account does not have a connection. AccountId:" << mAccountId;
        setStatus(Failed);
        scheduleDeletion();
        return;
    }

    switch(mProperties["chatType"].toUInt()) {
    case Tp::HandleTypeNone:
    case Tp::HandleTypeContact:
        startTextChat(account->account(), mProperties);
        break;
    case Tp::HandleTypeRoom:
        startTextChatRoom(account->account(), mProperties);
        break;
    default:
        qCritical() << "Chat type not supported";
    }
}

void ChatStartingJob::startTextChat(const Tp::AccountPtr &account, const QVariantMap &properties)
{
    qDebug() << __PRETTY_FUNCTION__;
    Tp::PendingChannelRequest *op = NULL;
    QStringList participants = properties["participantIds"].toStringList();
    switch(participants.size()) {
    case 0:
        qCritical() << "Error: No participant list provided";
        break;
    case 1:
        op = account->ensureTextChat(participants[0], QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler");
        break;
    default:
        op = account->createConferenceTextChat(QList<Tp::ChannelPtr>(), participants, QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler");
    }

    if (!op) {
        setStatus(Failed);
        scheduleDeletion();
        return;
    }

    connect(op, &Tp::PendingOperation::finished,
            this, &ChatStartingJob::onChannelRequestFinished);
}

void ChatStartingJob::startTextChatRoom(const Tp::AccountPtr &account, const QVariantMap &properties)
{
    qDebug() << __PRETTY_FUNCTION__;
    QString roomName = properties["threadId"].toString();

    // these properties are still not used
    //QString server = properties["Server"].toString();
    //QString creator = properties["Creator"].toString();

    QVariantMap request;
    Tp::PendingChannelRequest *op = NULL;
    if (roomName.isEmpty()) {
        request.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".ChannelType"), TP_QT_IFACE_CHANNEL_TYPE_TEXT);
        request.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandleType"), (uint) Tp::HandleTypeNone);
        QStringList initialInviteeIDs = properties["participantIds"].toStringList();
        if (!initialInviteeIDs.isEmpty()) {
            request.insert(TP_QT_IFACE_CHANNEL_INTERFACE_CONFERENCE + QLatin1String(".InitialInviteeIDs"), initialInviteeIDs);
        }
        // the presence of RoomName indicates the returned channel must be of type Room
        request.insert(TP_QT_IFACE_CHANNEL_INTERFACE_ROOM + QLatin1String(".RoomName"), QString());

        // TODO use the instance returned by createChanne() to track when the channel creation is finished
        op = account->createChannel(request, QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler");
    } else {
        op = account->ensureTextChatroom(roomName, QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler", request);
    }

    if (!op) {
        setStatus(Failed);
        scheduleDeletion();
        return;
    }
    connect(op, &Tp::PendingOperation::finished,
            this, &ChatStartingJob::onChannelRequestFinished);
}

Tp::TextChannelPtr ChatStartingJob::textChannel() const
{
    qDebug() << __PRETTY_FUNCTION__;
    return mTextChannel;
}

QString ChatStartingJob::channelObjectPath() const
{
    if (mTextChannel.isNull()) {
        return QString::null;
    }
    return mTextChannel->objectPath();
}

void ChatStartingJob::setTextChannel(Tp::TextChannelPtr channel)
{
    qDebug() << __PRETTY_FUNCTION__;
    mTextChannel = channel;
    Q_EMIT textChannelChanged();
}

void ChatStartingJob::onChannelRequestFinished(Tp::PendingOperation *op)
{
    qDebug() << __PRETTY_FUNCTION__;
    Status status;
    if (op->isError()) {
        status = Failed;
    } else {
        Tp::PendingChannelRequest *channelRequest = qobject_cast<Tp::PendingChannelRequest*>(op);
        if (!channelRequest) {
            status = Failed;
        } else {
            setTextChannel(Tp::TextChannelPtr::dynamicCast(channelRequest->channelRequest()->channel()));
            status = Finished;
        }
    }

    setStatus(status);
    scheduleDeletion();
}

