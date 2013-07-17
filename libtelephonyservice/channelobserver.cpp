/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "channelobserver.h"
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/ChannelClassSpecList>
#include <TelepathyQt/MethodInvocationContext>
#include <TelepathyQt/TextChannel>

ChannelObserver::ChannelObserver(QObject *parent) :
    QObject(parent), Tp::AbstractClientObserver(channelFilters(), true)
{
}

Tp::ChannelClassSpecList ChannelObserver::channelFilters() const
{
    Tp::ChannelClassSpecList specList;
    specList << Tp::ChannelClassSpec::audioCall();
    specList << Tp::ChannelClassSpec::textChat();

    return specList;
}

void ChannelObserver::observeChannels(const Tp::MethodInvocationContextPtr<> &context,
                                      const Tp::AccountPtr &account,
                                      const Tp::ConnectionPtr &connection,
                                      const QList<Tp::ChannelPtr> &channels,
                                      const Tp::ChannelDispatchOperationPtr &dispatchOperation,
                                      const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                                      const Tp::AbstractClientObserver::ObserverInfo &observerInfo)
{
    Q_UNUSED(account)
    Q_UNUSED(connection)
    Q_UNUSED(dispatchOperation)
    Q_UNUSED(requestsSatisfied)
    Q_UNUSED(observerInfo)

    Q_FOREACH (Tp::ChannelPtr channel, channels) {
        mContexts[channel.data()] = context;
        mChannels.append(channel);

        connect(channel.data(),
                SIGNAL(invalidated(Tp::DBusProxy*,const QString&, const QString&)),
                SLOT(onChannelInvalidated()));

        Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(channel);
        if (callChannel) {
            Tp::PendingReady *ready = callChannel->becomeReady(Tp::Features()
                                                               << Tp::CallChannel::FeatureCore
                                                               << Tp::CallChannel::FeatureCallMembers
                                                               << Tp::CallChannel::FeatureCallState
                                                               << Tp::CallChannel::FeatureContents
                                                               << Tp::CallChannel::FeatureLocalHoldState);
            connect(ready,
                    SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onCallChannelReady(Tp::PendingOperation*)));
            mReadyMap[ready] = callChannel;
        }

        Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(channel);
        if (textChannel) {
            Tp::PendingReady *ready = textChannel->becomeReady(Tp::Features()
                                                               << Tp::TextChannel::FeatureCore
                                                               << Tp::TextChannel::FeatureChatState
                                                               << Tp::TextChannel::FeatureMessageCapabilities
                                                               << Tp::TextChannel::FeatureMessageQueue
                                                               << Tp::TextChannel::FeatureMessageSentSignal);
            connect(ready,
                    SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onTextChannelReady(Tp::PendingOperation*)));
            mReadyMap[ready] = textChannel;
        }
    }
}

void ChannelObserver::onCallChannelReady(Tp::PendingOperation *op)
{
    Tp::PendingReady *ready = qobject_cast<Tp::PendingReady*>(op);
    if (!ready) {
        qCritical() << "Pending operation is not a pending ready:" << op;
        return;
    }

    if (!mReadyMap.contains(ready)) {
        qWarning() << "Pending ready finished but not on the map:" << ready;
        return;
    }

    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(mReadyMap[ready]);
    mReadyMap.remove(ready);

    if (!callChannel) {
        qWarning() << "Ready channel is not a call channel:" << callChannel;
        return;
    }

    // save the timestamp as a property in the call channel
    callChannel->setProperty("timestamp", QDateTime::currentDateTime());
    if (callChannel->callState() == Tp::CallStateActive) {
        callChannel->setProperty("activeTimestamp", QDateTime::currentDateTime());
    }

    Q_EMIT callChannelAvailable(callChannel);

    checkContextFinished(callChannel.data());
}

void ChannelObserver::onChannelInvalidated()
{
    Tp::ChannelPtr channel(qobject_cast<Tp::Channel*>(sender()));
    mChannels.removeAll(channel);
}

void ChannelObserver::onTextChannelReady(Tp::PendingOperation *op)
{
    Tp::PendingReady *ready = qobject_cast<Tp::PendingReady*>(op);
    if (!ready) {
        qCritical() << "Pending operation is not a pending ready:" << op;
        return;
    }

    if (!mReadyMap.contains(ready)) {
        qWarning() << "Pending ready finished but not on the map:" << ready;
        return;
    }

    Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(mReadyMap[ready]);
    mReadyMap.remove(ready);

    if (!textChannel) {
        qWarning() << "Ready channel is not a call channel:" << textChannel;
        return;
    }


    Q_EMIT textChannelAvailable(textChannel);
    checkContextFinished(textChannel.data());
}

void ChannelObserver::checkContextFinished(Tp::Channel *channel)
{
    if (!mContexts.contains(channel)) {
        qWarning() << "Context for channel not available:" << channel;
        return;
    }

    Tp::MethodInvocationContextPtr<> context = mContexts[channel];
    mContexts.remove(channel);

    // check if this is the last channel from the context
    Q_FOREACH(Tp::MethodInvocationContextPtr<> otherContext, mContexts.values()) {
        // if we find the context, just return from the function. We need to wait
        // for the other channels to become ready before setting the context finished
        if (otherContext == context) {
            return;
        }
    }

    context->setFinished();
}
