/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "channelhandler.h"
#include "chatmanager.h"
#include "telepathyhelper.h"
#include "config.h"

#include <TelepathyQt/MethodInvocationContext>
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/PendingReady>

ChannelHandler::ChannelHandler(QObject *parent)
    : QObject(parent), Tp::AbstractClientHandler(channelFilters())
{
}

bool ChannelHandler::bypassApproval() const
{
    return false;
}

void ChannelHandler::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                               const Tp::AccountPtr &account,
                               const Tp::ConnectionPtr &connection,
                               const QList<Tp::ChannelPtr> &channels,
                               const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                               const QDateTime &userActionTime,
                               const Tp::AbstractClientHandler::HandlerInfo &handlerInfo)
{
    // if the class is not in the telephony application, we should only handle the channels that
    // were requested by this instance.
    if (!isTelephonyApplicationInstance()) {
        QString handler = TelepathyHelper::instance()->channelHandler()->property("clientName").toString();
        Q_FOREACH(Tp::ChannelRequestPtr channelRequest, requestsSatisfied) {
            if (channelRequest->preferredHandler() != handler) {
                context->setFinishedWithError(TP_QT_ERROR_REJECTED,
                                              "The channel should be handled by org.freedesktop.Telepathy.Client.TelephonyApp");
            }
        }
    }

    Q_FOREACH(const Tp::ChannelPtr channel, channels) {
        Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(channel);
        if (textChannel) {
            Tp::PendingReady *pr = textChannel->becomeReady(Tp::Features()
                                                         << Tp::TextChannel::FeatureCore
                                                         << Tp::TextChannel::FeatureChatState
                                                         << Tp::TextChannel::FeatureMessageCapabilities
                                                         << Tp::TextChannel::FeatureMessageQueue
                                                         << Tp::TextChannel::FeatureMessageSentSignal);

            connect(pr, SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onTextChannelReady(Tp::PendingOperation*)));

            connect(textChannel.data(), SIGNAL(messageReceived(Tp::ReceivedMessage)),
                    ChatManager::instance(), SLOT(onMessageReceived(Tp::ReceivedMessage)));
            mReadyRequests[pr] = textChannel;
            continue;
        }

        Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(channel);
        if (callChannel) {
            Tp::PendingReady *pr = callChannel->becomeReady(Tp::Features()
                                             << Tp::CallChannel::FeatureCore
                                             << Tp::CallChannel::FeatureCallState
                                             << Tp::CallChannel::FeatureContents);
            connect(pr, SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onCallChannelReady(Tp::PendingOperation*)));
            mReadyRequests[pr] = callChannel;
            continue;
        }

    }
    context->setFinished();
}

Tp::ChannelClassSpecList ChannelHandler::channelFilters()
{
    Tp::ChannelClassSpecList specList;
    specList << Tp::ChannelClassSpec::audioCall();
    specList << Tp::ChannelClassSpec::textChat();

    return specList;
}

void ChannelHandler::onTextChannelReady(Tp::PendingOperation *op)
{
    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady*>(op);

    if (!pr) {
        qCritical() << "The pending object is not a Tp::PendingReady";
        return;
    }

    Tp::ChannelPtr channel = mReadyRequests[pr];
    Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(channel);

    if(!textChannel) {
        qCritical() << "The saved channel is not a Tp::TextChannel";
        return;
    }

    mReadyRequests.remove(pr);

    Q_EMIT textChannelAvailable(textChannel);
}

void ChannelHandler::onCallChannelReady(Tp::PendingOperation *op)
{
    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady*>(op);

    if (!pr) {
        qCritical() << "The pending object is not a Tp::PendingReady";
        return;
    }

    Tp::ChannelPtr channel = mReadyRequests[pr];
    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(channel);

    if(!callChannel) {
        qCritical() << "The saved channel is not a Tp::CallChannel";
        return;
    }

    mReadyRequests.remove(pr);

    Q_EMIT callChannelAvailable(callChannel);
}
