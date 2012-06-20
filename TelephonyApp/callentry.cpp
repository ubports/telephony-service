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

#define TP_UFA_DBUS_ADDRESS "org.freedesktop.Telepathy.Connection.ufa.ufa.ufa"
#define TP_UFA_DBUS_MUTE_FACE "org.freedesktop.Telepathy.Call1.Interface.Mute"

CallEntry::CallEntry(const QString &contactId, const Tp::CallChannelPtr &channel, QObject *parent) :
    QObject(parent),
    mChannel(channel),
    mContactId(contactId),
    mMuteInterface(TP_UFA_DBUS_ADDRESS, channel->objectPath(), TP_UFA_DBUS_MUTE_FACE)
{
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
            SIGNAL(MuteStateChanged(uint32)),
            SIGNAL(mutedChanged()));
}

bool CallEntry::isHeld() const
{
    return (mChannel->localHoldState() == Tp::LocalHoldStateHeld);
}

QString CallEntry::contactId() const
{
    return mContactId;
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
