/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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
#define TELEPATHY_SPEAKER_IFACE "com.canonical.Telephony.Speaker"
#define DBUS_PROPERTIES_IFACE "org.freedesktop.DBus.Properties"
#define PROPERTY_SPEAKERMODE "SpeakerMode"

CallEntry::CallEntry(const Tp::CallChannelPtr &channel, QObject *parent) :
    QObject(parent),
    mChannel(channel),
    mVoicemail(false),
    mLocalMuteState(false),
    mElapsedTime(QTime::currentTime()),
    mMuteInterface(channel->busName(), channel->objectPath(), TELEPATHY_MUTE_IFACE),
    mSpeakerInterface(channel->busName(), channel->objectPath(), TELEPATHY_SPEAKER_IFACE),
    mHasSpeakerProperty(false),
    mSpeakerMode(false)
{
    setupCallChannel();

    Q_EMIT incomingChanged();
}

void CallEntry::onSpeakerChanged(bool active)
{
    mSpeakerMode = active;
    Q_EMIT speakerChanged();
}

void CallEntry::setupCallChannel()
{
    connect(mChannel.data(),
            SIGNAL(callStateChanged(Tp::CallState)),
            SLOT(onCallStateChanged(Tp::CallState)));
    connect(mChannel.data(),
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

    onCallStateChanged(mChannel->callState());

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

     reply = callChannelIface.call("GetAll", TELEPATHY_SPEAKER_IFACE);
     args = reply.arguments();
     QMap<QString, QVariant> map2 = qdbus_cast<QMap<QString, QVariant> >(args[0]);

     mProperties.clear();
     QMapIterator<QString, QVariant> i(map);
     while(i.hasNext()) {
         i.next();
         mProperties[i.key()] = i.value();
     }
     QMapIterator<QString, QVariant> i2(map2);
     while(i2.hasNext()) {
         i2.next();
         mProperties[i2.key()] = i2.value();
     }

     onSpeakerChanged(mProperties[PROPERTY_SPEAKERMODE].toBool());
}

bool CallEntry::dialing() const
{
    return !incoming() && (mChannel->callState() == Tp::CallStateInitialised);
}

bool CallEntry::incoming() const
{
    return mChannel->initiatorContact() != TelepathyHelper::instance()->account()->connection()->selfContact();
}

bool CallEntry::ringing() const
{
    return mChannel->callFlags() & Tp::CallFlagLocallyRinging;
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
    return mContact.detail<QContactDisplayLabel>().label();
}

QString CallEntry::contactAvatar() const
{
    QContactAvatar avatar = mContact.detail<QContactAvatar>();
    return avatar.imageUrl().toString();
}

void CallEntry::sendDTMF(const QString &key)
{
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->call("SendDTMF", mChannel->objectPath(), key);
}

void CallEntry::endCall()
{
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->call("HangUpCall", mChannel->objectPath());
}

Tp::CallChannelPtr CallEntry::channel() const
{
    return mChannel;
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
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->call("SetHold", mChannel->objectPath(), hold);
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
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->call("SetMuted", mChannel->objectPath(), value);
}

void CallEntry::onCallStateChanged(Tp::CallState state)
{
    switch (state) {
    case Tp::CallStateEnded:
        Q_EMIT callEnded();
        break;
    case Tp::CallStateActive:
        mChannel->setProperty("activeTimestamp", QDateTime::currentDateTime());
        startTimer(1000);
        mElapsedTime.start();
        Q_EMIT callActive();
        Q_EMIT activeChanged();
        break;
    }

    Q_EMIT dialingChanged();
}

void CallEntry::onCallFlagsChanged(Tp::CallFlags flags)
{
    Q_UNUSED(flags)

    Q_EMIT ringingChanged();
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

    return false;
}

void CallEntry::setSpeaker(bool speaker)
{
    if (!mHasSpeakerProperty) {
        return;
    }

    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->call("SetSpeakerMode", mChannel->objectPath(), speaker);
}

