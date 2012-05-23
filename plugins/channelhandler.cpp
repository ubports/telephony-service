/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "channelhandler.h"

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
    foreach(const Tp::ChannelPtr channel, channels) {
        Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(channel);
        if (textChannel) {
            qDebug() << "ChannelHandler: got text channel";

            Tp::PendingReady *pr = textChannel->becomeReady(Tp::Features()
                                                         << Tp::TextChannel::FeatureCore
                                                         << Tp::TextChannel::FeatureChatState
                                                         << Tp::TextChannel::FeatureMessageCapabilities
                                                         << Tp::TextChannel::FeatureMessageQueue
                                                         << Tp::TextChannel::FeatureMessageSentSignal);

            connect(pr, SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onTextChannelReady(Tp::PendingOperation*)));
            mReadyRequests[pr] = textChannel;
            continue;
        }

        Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(channel);
        if (callChannel) {
            // TODO: implement
            qDebug() << "ChannelHandler: got call channel";
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
    Q_ASSERT(pr);

    Tp::ChannelPtr channel = mReadyRequests[pr];
    Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(channel);
    Q_ASSERT(textChannel);


    mReadyRequests.remove(pr);

    emit textChannelAvailable(textChannel);
}
