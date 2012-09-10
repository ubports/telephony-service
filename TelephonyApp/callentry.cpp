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
#include "telepathyhelper.h"
#include <QTime>
#include <QContactAvatar>
#include <TelepathyQt/Contact>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/Connection>

#define TELEPATHY_MUTE_IFACE "org.freedesktop.Telepathy.Call1.Interface.Mute"
#define TELEPATHY_CALL_IFACE "org.freedesktop.Telepathy.Channel.Type.Call1"
#define DBUS_PROPERTIES_IFACE "org.freedesktop.DBus.Properties"
#define PROPERTY_SPEAKERMODE "SpeakerMode"

CallEntry::CallEntry(const Tp::CallChannelPtr &channel, QObject *parent) :
    QObject(parent),
    mChannel(channel),
    mVoicemail(false),
    mLocalMuteState(false),
    mElapsedTime(QTime::currentTime()),
    mMuteInterface(channel->busName(), channel->objectPath(), TELEPATHY_MUTE_IFACE),
    mSpeakerInterface(channel->busName(), channel->objectPath(), TELEPATHY_CALL_IFACE),
    mChannelReady(false),
    mHasSpeakerProperty(false),
    mSpeakerMode(false)
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

    connect(&mMuteInterface,
            SIGNAL(MuteStateChanged(uint)),
            SLOT(onMutedChanged(uint)));

    refreshProperties();

    mHasSpeakerProperty = mProperties.contains(PROPERTY_SPEAKERMODE);
    if (mHasSpeakerProperty) {
        connect(&mSpeakerInterface, SIGNAL(SpeakerChanged(bool)), SLOT(onSpeakerChanged(bool)));
    }
}

void CallEntry::onSpeakerChanged(bool active)
{
    mSpeakerMode = active;
    emit speakerChanged();
}

void CallEntry::timerEvent(QTimerEvent *event)
{
    Q_EMIT elapsedTimeChanged();
}

void CallEntry::refreshProperties()
{
     QDBusInterface callChannelIface(mChannel->busName(), mChannel->objectPath(), DBUS_PROPERTIES_IFACE);
     QDBusMessage reply = callChannelIface.call("GetAll", TELEPATHY_CALL_IFACE);
     QVariantList args = reply.arguments();
     QMap<QString, QVariant> map = qdbus_cast<QMap<QString, QVariant> >(args[0]);
     mProperties.clear();
     QMapIterator<QString, QVariant> i(map);
     while(i.hasNext()) {
         i.next();
         mProperties[i.key()] = i.value();
     }

     onSpeakerChanged(mProperties[PROPERTY_SPEAKERMODE].toBool());
}

bool CallEntry::dialing() const
{
    if (!mChannelReady) {
        return false;
    }

    bool isOutgoing = mChannel->initiatorContact() == TelepathyHelper::instance()->account()->connection()->selfContact();
      
    return isOutgoing && (mChannel->callState() == Tp::CallStateInitialised);
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
    Q_EMIT callEnded();
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
    Q_EMIT mutedChanged();
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

    switch(mChannel->callState()) {
    case Tp::CallStateActive: 
        // start timer if this call is already active
        startTimer(1000);
        mElapsedTime.start();
        Q_EMIT callActive();
        break;
    case Tp::CallStateInitialised:
        Q_EMIT dialingChanged();
    default:
        // accept the call if it was not accepted yet
        mChannel->accept();
    }

    mChannelReady = true;

    ContactEntry *entry = ContactModel::instance()->contactFromPhoneNumber(mChannel->targetContact()->id());
    if (entry) {
        mContact = entry->contact();
        Q_EMIT contactAliasChanged();
        Q_EMIT contactAvatarChanged();
    }

    Q_EMIT heldChanged();
    Q_EMIT phoneNumberChanged();
    Q_EMIT dialingChanged();
}

void CallEntry::onCallStateChanged(Tp::CallState state)
{
    if (state == Tp::CallStateEnded) {
        endCall();
    } else if (state == Tp::CallStateActive) {
        startTimer(1000);
        mElapsedTime.start();
        Q_EMIT callActive();
    }
    Q_EMIT dialingChanged();
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
    Q_EMIT voicemailChanged();
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

bool CallEntry::isSpeakerOn()
{
    if (mHasSpeakerProperty) {
        return mSpeakerMode;
    }
}

void CallEntry::setSpeaker(bool speaker)
{
    if (mHasSpeakerProperty) {
        mSpeakerInterface.call("turnOnSpeaker", speaker);
    }
}

