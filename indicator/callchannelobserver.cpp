/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#include "callchannelobserver.h"
#include "callnotification.h"
#include "contactwatcher.h"
#include "messagingmenu.h"
#include "metrics.h"
#include "telepathyhelper.h"
#include "accountentry.h"
#include <TelepathyQt/Contact>

CallChannelObserver::CallChannelObserver(QObject *parent) :
    QObject(parent)
{
}

void CallChannelObserver::onCallChannelAvailable(Tp::CallChannelPtr callChannel)
{
    // save the timestamp as a property in the call channel
    callChannel->setProperty("timestamp", QDateTime::currentDateTime());
    if (callChannel->callState() == Tp::CallStateActive) {
        callChannel->setProperty("activeTimestamp", QDateTime::currentDateTime());
    }


    connect(callChannel.data(),
                SIGNAL(callStateChanged(Tp::CallState)),
                SLOT(onCallStateChanged(Tp::CallState)));
    connect(callChannel.data(),
            SIGNAL(localHoldStateChanged(Tp::LocalHoldState,Tp::LocalHoldStateReason)),
            SLOT(onHoldChanged()));

    mChannels.append(callChannel);
    if (callChannel->isReady(Tp::CallChannel::FeatureCallState)) {
        qDebug() << "BLABLA call state was ready:" << callChannel->callState();
        mCallStates[callChannel.data()] = callChannel->callState();
    } else {
        connect(callChannel->becomeReady(Tp::CallChannel::FeatureCallState), &Tp::PendingReady::finished, [&](){
            qDebug() << "BLABLA call state was not ready, but it is now:" << callChannel->callState();
            mCallStates[callChannel.data()] = callChannel->callState();
        });
    }
}

void CallChannelObserver::onCallStateChanged(Tp::CallState state)
{
    Tp::CallChannelPtr channel(qobject_cast<Tp::CallChannel*>(sender()));
    if (!channel) {
        return;
    }

    AccountEntry *accountEntry = TelepathyHelper::instance()->accountForConnection(channel->connection());
    if (!accountEntry) {
        return;
    }

    bool incoming = channel->initiatorContact() != accountEntry->account()->connection()->selfContact();
    bool missed = incoming && channel->callStateReason().reason == Tp::CallStateChangeReasonNoAnswer;
    QDateTime activeTimestamp = channel->property("activeTimestamp").toDateTime();

    switch (state) {
    case Tp::CallStateEnded:
        Q_EMIT callEnded(channel);

        // if the missed flag is false, we still have to check if transitioning directly from Initialized to Ended
        if (!missed && incoming) {
            missed = mCallStates[channel.data()] == Tp::CallStateInitialised;
        }

        // add the missed call to the messaging menu
        if (missed) {
            // FIXME: handle conf call
            MessagingMenu::instance()->addCall(ContactWatcher::normalizeIdentifier(channel->targetContact()->id()), accountEntry->accountId(), QDateTime::currentDateTime());
        } else {
            // and show a notification
            // FIXME: handle conf call
            CallNotification::instance()->showNotificationForCall(QStringList() << ContactWatcher::normalizeIdentifier(channel->targetContact()->id()), CallNotification::CallEnded);
        }

        mCallStates.remove(channel.data());
        mChannels.removeOne(channel);

        // update the metrics
        Metrics::instance()->increment(incoming ? Metrics::IncomingCalls : Metrics::OutgoingCalls);
        if (activeTimestamp.isValid()) {
            double minutes = activeTimestamp.secsTo(QDateTime::currentDateTime()) / 60.;
            Metrics::instance()->increment(Metrics::CallDurations, qRound(minutes * 100) / 100);
        }
        break;
    case Tp::CallStateActive:
        channel->setProperty("activeTimestamp", QDateTime::currentDateTime());
        break;
    }
    mCallStates[channel.data()] = state;
}

void CallChannelObserver::onHoldChanged()
{
    Tp::CallChannelPtr channel(qobject_cast<Tp::CallChannel*>(sender()));
    if (!channel) {
        return;
    }

    if (channel->localHoldState() == Tp::LocalHoldStateHeld) {
        // FIXME: handle conf call
        CallNotification::instance()->showNotificationForCall(QStringList() << channel->targetContact()->id(), CallNotification::CallHeld);
    }
}
