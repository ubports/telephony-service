/*
 * Copyright (C) 2012-2017 Canonical, Ltd.
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

#include "handler.h"
#include "accountentry.h"
#include "protocolmanager.h"
#include "telepathyhelper.h"

#include <TelepathyQt/MethodInvocationContext>
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/PendingReady>

Handler::Handler(QObject *parent)
    : QObject(parent), Tp::AbstractClientHandler(channelFilters(), capabilities())
{
}

bool Handler::bypassApproval() const
{
    return false;
}

void Handler::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                             const Tp::AccountPtr &account,
                             const Tp::ConnectionPtr &connection,
                             const QList<Tp::ChannelPtr> &channels,
                             const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                             const QDateTime &userActionTime,
                             const Tp::AbstractClientHandler::HandlerInfo &handlerInfo)
{
    Q_UNUSED(connection)
    Q_UNUSED(requestsSatisfied)
    Q_UNUSED(userActionTime)
    Q_UNUSED(handlerInfo)

    if (!ProtocolManager::instance()->isProtocolSupported(account->protocolName())) {
        context->setFinishedWithError(TP_QT_ERROR_NOT_CAPABLE, "The account for this request is not supported.");
        return;
    }


    Q_FOREACH(const Tp::ChannelPtr channel, channels) {
        mContexts[channel.data()] = context;
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
}

Tp::ChannelClassSpecList Handler::channelFilters()
{
    Tp::ChannelClassSpecList specList;
    specList << TelepathyHelper::audioConferenceSpec();
    specList << Tp::ChannelClassSpec::audioCall();
    specList << Tp::ChannelClassSpec::textChat();
    specList << Tp::ChannelClassSpec::textChatroom();
    specList << Tp::ChannelClassSpec::unnamedTextChat();

    QVariantMap props;
    props[TP_QT_IFACE_CHANNEL_TYPE_CALL + ".InitialAudio"] = true;
    specList << Tp::ChannelClassSpec::audioCall(props);

    return specList;
}

Tp::AbstractClientHandler::Capabilities Handler::capabilities()
{
    QStringList caps;
    caps << TP_QT_IFACE_CHANNEL_TYPE_CALL + "/shm"
         << TP_QT_IFACE_CHANNEL_TYPE_CALL + "/ice"
         << TP_QT_IFACE_CHANNEL_TYPE_CALL + "/gtalk-p2p";
    return Tp::AbstractClientHandler::Capabilities(caps);
}

void Handler::onTextChannelReady(Tp::PendingOperation *op)
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
    Tp::MethodInvocationContextPtr<> context = mContexts.take(textChannel.data());
    if (context) {
        context->setFinished();
    }

    Q_EMIT textChannelAvailable(textChannel);
}

void Handler::onCallChannelReady(Tp::PendingOperation *op)
{
    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady*>(op);

    if (!pr) {
        qCritical() << "The pending object is not a Tp::PendingReady";
        return;
    }

    Tp::ChannelPtr channel = mReadyRequests.take(pr);
    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(channel);
    Tp::MethodInvocationContextPtr<> context = mContexts.take(channel.data());

    if(!callChannel) {
        if (context) {
            context->setFinishedWithError(TP_QT_ERROR_CONFUSED, "Channel was not a call channel");
        }
        qCritical() << "The saved channel is not a Tp::CallChannel";
        return;
    }

    // if the call is neither Accepted nor Active, it means it got dispatched directly to the handler without passing
    // through any approver. For phone calls, this would mean calls getting auto-accepted which is not desirable
    // so we return an error here
    bool incoming;
    AccountEntry *accountEntry = TelepathyHelper::instance()->accountForConnection(callChannel->connection());
    if (accountEntry && !callChannel->initiatorContact().isNull()) {
        incoming = callChannel->initiatorContact() != accountEntry->account()->connection()->selfContact();
    } else {
        incoming = !callChannel->isRequested();
    }

    if (incoming && callChannel->callState() != Tp::CallStateAccepted && callChannel->callState() != Tp::CallStateActive) {
        qWarning() << "Available channel was not approved by telephony-service-approver, ignoring it.";
        if (context) {
            context->setFinishedWithError(TP_QT_ERROR_NOT_CAPABLE, "Only channels approved and accepted by telephony-service-approver are supported");
        }
        return;
    }

    if (context) {
        context->setFinished();
    }

    Q_EMIT callChannelAvailable(callChannel);
}
