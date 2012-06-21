/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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

#include "callentry.h"
#include <TelepathyQt/Contact>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/Connection>

#define TP_UFA_DBUS_MUTE_FACE "org.freedesktop.Telepathy.Call1.Interface.Mute"

CallEntry::CallEntry(const Tp::CallChannelPtr &channel, QObject *parent) :
    QObject(parent),
    mChannel(channel),
    mMuteInterface(channel->connectibusName(), channel->objectPath(), TP_UFA_DBUS_MUTE_FACE)
{
    connect(mChannel->becomeReady(Tp::Features()
                                  << Tp::CallChannel::FeatureCore
                                  << Tp::CallChannel::FeatureCallMembers
                                  << Tp::CallChannel::FeatureCallState
                                  << Tp::CallChannel::FeatureContents
                                  << Tp::CallChannel::FeatureLocalHoldState),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onChannelReady(Tp::PendingOperation*)));
    connect(channel.data(),
            SIGNAL(callStateChanged(Tp::CallState)),
            SLOT(onCallStateChanged(Tp::CallState)));
    connect(channel.data(),
            SIGNAL(callFlagsChanged(Tp::CallFlags)),
            SLOT(onCallFlagsChanged(Tp::CallFlags)));
    connect(mChannel.data(),
            SIGNAL(localHoldStateChanged(Tp::LocalHoldState,Tp::LocalHoldStateReason)),
            SIGNAL(heldChanged()));
    channel->accept();

    connect(&mMuteInterface,
            SIGNAL(MuteStateChanged(uint)),
            SIGNAL(mutedChanged()));
}

QString CallEntry::phoneNumber() const
{
    if (!mChannel->actualFeatures().contains(Tp::CallChannel::FeatureCore)) {
        return "";
    }
    return mChannel->targetContact()->id();
}

void CallEntry::sendDTMF(const QString &key)
{
    foreach(const Tp::CallContentPtr &content, mChannel->contents()) {
        if (content->supportsDTMF()) {
            bool ok;
            Tp::DTMFEvent event = (Tp::DTMFEvent)key.toInt(&ok);
            if (!ok) {
                 if (!key.compare("*")) {
                     event = Tp::DTMFEventAsterisk;
                 } else if (!key.compare("#")) {
                     event = Tp::DTMFEventHash;
                 } else {
                     qWarning() << "Tone not recognized. DTMF failed";
                     return;
                 }
            }
            content->startDTMFTone(event);
        }
    }
}

void CallEntry::endCall()
{
    mChannel->hangup();
    mChannel->requestClose();
}

bool CallEntry::isHeld() const
{
    if (!mChannel->actualFeatures().contains(Tp::CallChannel::FeatureLocalHoldState)) {
        return false;
    }
    return (mChannel->localHoldState() == Tp::LocalHoldStateHeld);
}


void CallEntry::setHold(bool hold)
{
    mChannel->requestHold(hold);
}

bool CallEntry::isMuted() const
{
    // Replace this by a Mute interface method call when it
    // becomes available in telepathy-qt
    return (mMuteInterface.property("LocalMuteState") == 1);
}

void CallEntry::setMute(bool value)
{
    // Replace this by a Mute interface method call when it
    // becomes available in telepathy-qt
    mMuteInterface.call("RequestMuted", value);
}

void CallEntry::onChannelReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        qWarning() << "PendingOperation finished with error:" << op->errorName() << op->errorMessage();
    }

    emit heldChanged();
    emit phoneNumberChanged();
}

void CallEntry::onCallStateChanged(Tp::CallState state)
{
    if (state == Tp::CallStateEnded) {
        endCall();
        emit callEnded();
    }
}

void CallEntry::onCallFlagsChanged(Tp::CallFlags flags)
{
    // TODO: handle ringing
}
