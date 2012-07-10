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
#include "contactentry.h"
#include "contactmodel.h"
#include <QTime>
#include <QContactAvatar>
#include <TelepathyQt/Contact>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/Connection>

#define TP_UFA_DBUS_MUTE_FACE "org.freedesktop.Telepathy.Call1.Interface.Mute"

CallEntry::CallEntry(const Tp::CallChannelPtr &channel, QObject *parent) :
    QObject(parent),
    mChannel(channel),
    mVoicemail(false),
    mLocalMuteState(false),
    mElapsedTime(QTime::currentTime()),
    mMuteInterface(channel->busName(), channel->objectPath(), TP_UFA_DBUS_MUTE_FACE)
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
            SLOT(onMutedChanged(uint)));

    // start timer if this call is already active
    if(mChannel->callState() == Tp::CallStateActive) {
        startTimer(1000);
        mElapsedTime.start();
        Q_EMIT callActive();
    }

}

void CallEntry::timerEvent(QTimerEvent *event)
{
    emit elapsedTimeChanged();
}

QString CallEntry::phoneNumber() const
{
    if (!mChannel->actualFeatures().contains(Tp::CallChannel::FeatureCore)) {
        return "";
    }
    return mChannel->targetContact()->id();
}

QString CallEntry::contactAlias() const
{
    return mContact.displayLabel();
}

QString CallEntry::contactAvatar() const
{
    QContactAvatar avatar = mContact.detail<QContactAvatar>();
    return avatar.imageUrl().toString();
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
    connect(mChannel->hangup(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onCallHangupFinished(Tp::PendingOperation*)));
    emit callEnded();
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

void CallEntry::onMutedChanged(uint state)
{
    // Replace this by a Mute interface method call when it
    // becomes available in telepathy-qt
    mLocalMuteState = (state == 1);
    emit mutedChanged();
}

bool CallEntry::isMuted() const
{
    // Replace this by a Mute interface method call when it
    // becomes available in telepathy-qt
    return mLocalMuteState;
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

    ContactEntry *entry = ContactModel::instance()->contactFromPhoneNumber(mChannel->targetContact()->id());
    if (entry) {
        mContact = entry->contact();
        emit contactAliasChanged();
        emit contactAvatarChanged();
    }

    emit heldChanged();
    emit phoneNumberChanged();
}

void CallEntry::onCallStateChanged(Tp::CallState state)
{
    if (state == Tp::CallStateEnded) {
        endCall();
    } else if (state == Tp::CallStateActive) {
        startTimer(1000);
        mElapsedTime.start();
        emit callActive();
    }
}

void CallEntry::onCallFlagsChanged(Tp::CallFlags flags)
{
    // TODO: handle ringing
}

void CallEntry::onCallHangupFinished(Tp::PendingOperation *op)
{
    mChannel->requestClose();
}

void CallEntry::setVoicemail(bool voicemail)
{
    mVoicemail = voicemail;
    emit voicemailChanged();
}

bool CallEntry::isVoicemail() const
{
    return mVoicemail;
}

int CallEntry::elapsedTime() const
{
    return mElapsedTime.secsTo(QTime::currentTime());
}

bool CallEntry::isActive() const
{
    return (mChannel->callState() == Tp::CallStateActive);
}

