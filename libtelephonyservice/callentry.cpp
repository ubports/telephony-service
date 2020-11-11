/*
 * Copyright (C) 2012-2017 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "callentry.h"
#include "callmanager.h"
#include "telepathyhelper.h"
#include "accountentry.h"
#include "ofonoaccountentry.h"

#include <QDBusReply>
#include <QTime>
#include <TelepathyQt/Contact>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/Connection>

#define TELEPATHY_MUTE_IFACE "org.freedesktop.Telepathy.Call1.Interface.Mute"
#define TELEPATHY_CALL_IFACE "org.freedesktop.Telepathy.Channel.Type.Call1"
#define DBUS_PROPERTIES_IFACE "org.freedesktop.DBus.Properties"
#define PROPERTY_AUDIO_OUTPUTS "AudioOutputs"
#define PROPERTY_ACTIVE_AUDIO_OUTPUT "ActiveAudioOutput"

CallEntry::CallEntry(const Tp::CallChannelPtr &channel, QObject *parent) :
    QObject(parent),
    mChannel(channel),
    mVoicemail(false),
    mLocalMuteState(false),
    mMuteInterface(channel->busName(), channel->objectPath(), TELEPATHY_MUTE_IFACE)
{
    qRegisterMetaType<AudioOutputDBus>();
    qRegisterMetaType<AudioOutputDBusList>();

    qDBusRegisterMetaType<AudioOutputDBus>();
    qDBusRegisterMetaType<AudioOutputDBusList>();

    mAccount = TelepathyHelper::instance()->accountForConnection(mChannel->connection());
    setupCallChannel();

    // connect to the DBus signal
    connect(TelepathyHelper::instance()->handlerInterface(),
            SIGNAL(CallPropertiesChanged(QString, QVariantMap)),
            SLOT(onCallPropertiesChanged(QString,QVariantMap)));

    connect(TelepathyHelper::instance()->handlerInterface(),
            SIGNAL(CallHoldingFailed(QString)),
            SLOT(onCallHoldingFailed(QString)));

    connect(TelepathyHelper::instance()->handlerInterface(),
            SIGNAL(ActiveAudioOutputChanged(QString)),
            SLOT(onActiveAudioOutputChanged(QString)));

    QDBusConnection::sessionBus().connect(TelepathyHelper::instance()->handlerInterface()->service(),
                                          TelepathyHelper::instance()->handlerInterface()->path(),
                                          TelepathyHelper::instance()->handlerInterface()->interface(),
                                          "AudioOutputsChanged",
                                          this,
                                          SLOT(onAudioOutputsChanged(AudioOutputDBusList)));

    // in case the account is an ofono account, we can check the voicemail number
    OfonoAccountEntry *ofonoAccount = qobject_cast<OfonoAccountEntry*>(mAccount);
    if (ofonoAccount && !ofonoAccount->voicemailNumber().isEmpty()) {
        setVoicemail(phoneNumber() == ofonoAccount->voicemailNumber());
    }

    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();

    QDBusMessage reply = phoneAppHandler->call(PROPERTY_AUDIO_OUTPUTS);
    AudioOutputDBusList audioOutputList = qdbus_cast<AudioOutputDBusList>(reply.arguments().first());
    onAudioOutputsChanged(audioOutputList);

    QString activeAudioOutput = phoneAppHandler->property(PROPERTY_ACTIVE_AUDIO_OUTPUT).toString();
    onActiveAudioOutputChanged(activeAudioOutput);

    Q_EMIT incomingChanged();
}

void CallEntry::onAudioOutputsChanged(const AudioOutputDBusList &outputs)
{
    mAudioOutputs.clear();
    while (!mAudioOutputs.isEmpty()) {
        mAudioOutputs.takeFirst()->deleteLater();
    }
    Q_FOREACH(const AudioOutputDBus &output, outputs) {
        mAudioOutputs.append(new AudioOutput(output.id, output.name, output.type, this));
    }
    Q_EMIT audioOutputsChanged();
}

QString CallEntry::activeAudioOutput() const
{
    return mActiveAudioOutput;
}

void CallEntry::setActiveAudioOutput(const QString &id)
{
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->setProperty("ActiveAudioOutput", id);
}

void CallEntry::onActiveAudioOutputChanged(const QString &id)
{
    mActiveAudioOutput = id;
    Q_EMIT activeAudioOutputChanged();
}

void CallEntry::onCallPropertiesChanged(const QString &objectPath, const QVariantMap &properties)
{
    if (objectPath != mChannel->objectPath()) {
        return;
    }
    updateChannelProperties(properties);
}

void CallEntry::onConferenceChannelMerged(const Tp::ChannelPtr &channel)
{
    QList<CallEntry*> entries = CallManager::instance()->takeCalls(QList<Tp::ChannelPtr>() << channel);
    if (entries.isEmpty()) {
        qWarning() << "Could not find the call that was just merged.";
        return;
    }

    CallEntry *entry = entries.first();
    connect(entry,
            SIGNAL(callEnded()),
            SLOT(onInternalCallEnded()));
    mCalls.append(entry);
    Q_EMIT callsChanged();
}

void CallEntry::onConferenceChannelRemoved(const Tp::ChannelPtr &channel, const Tp::Channel::GroupMemberChangeDetails &details)
{
    Q_FOREACH(CallEntry *entry, mCalls) {
        Tp::ChannelPtr entryChannel = entry->channel();
        if (entryChannel == channel) {
            CallManager::instance()->addCalls(QList<CallEntry*>() << entry);
            mCalls.removeAll(entry);
            entry->disconnect(this);
            Q_EMIT callsChanged();
            break;
        }
    }
}

void CallEntry::onInternalCallEnded()
{
    CallEntry *entry = qobject_cast<CallEntry*>(sender());
    mCalls.removeAll(entry);
    Q_EMIT callsChanged();
    entry->deleteLater();
}

void CallEntry::onCallHoldingFailed(const QString &objectPath)
{
    if (objectPath != mChannel->objectPath()) {
        return;
    }

    // make sure we get the hold state again
    Q_EMIT heldChanged();
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
            SLOT(onCallLocalHoldStateChanged(Tp::LocalHoldState,Tp::LocalHoldStateReason)));

    mLocalMuteState = mMuteInterface.property("LocalMuteState") == 1;
    connect(&mMuteInterface,
            SIGNAL(MuteStateChanged(uint)),
            SLOT(onMutedChanged(uint)));

    if (mChannel->isConference()) {
        connect(mChannel.data(),
                SIGNAL(conferenceChannelMerged(Tp::ChannelPtr)),
                SLOT(onConferenceChannelMerged(Tp::ChannelPtr)));
        connect(mChannel.data(),
                SIGNAL(conferenceChannelRemoved(Tp::ChannelPtr, Tp::Channel::GroupMemberChangeDetails)),
                SLOT(onConferenceChannelRemoved(Tp::ChannelPtr,Tp::Channel::GroupMemberChangeDetails)));
    }

    refreshProperties();

    onCallStateChanged(mChannel->callState());

    Q_EMIT heldChanged();
    Q_EMIT phoneNumberChanged();
    Q_EMIT dialingChanged();
}

void CallEntry::updateChannelProperties(const QVariantMap &properties)
{
    QVariantMap props = properties;
    // fetch the properties if the map is empty
    if (props.isEmpty()) {
        QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
        QDBusReply<QVariantMap> reply = phoneAppHandler->call("GetCallProperties", mChannel->objectPath());
        if (!reply.isValid()) {
            return;
        }

        props = reply.value();
    }

    QDateTime timestamp;
    if (props.contains("timestamp")) {
        props["timestamp"].value<QDBusArgument>() >> timestamp;
    }
    if (props.contains("activeTimestamp")) {
        props["activeTimestamp"].value<QDBusArgument>() >> mActiveTimestamp;
    }

    mChannel->setProperty("dtmfString", props["dtmfString"]);
    mChannel->setProperty("timestamp", timestamp);
    mChannel->setProperty("activeTimestamp", mActiveTimestamp);

    Q_EMIT dtmfStringChanged();
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
}

bool CallEntry::dialing() const
{
    return !incoming() && (mChannel->callState() == Tp::CallStateInitialised);
}

bool CallEntry::incoming() const
{
    bool isIncoming;

    if (mAccount && !mChannel->initiatorContact().isNull()) {
        isIncoming = mChannel->initiatorContact() != mAccount->account()->connection()->selfContact();
    } else {
        isIncoming = !mChannel->isRequested();
    }

    return isIncoming;
}

bool CallEntry::ringing() const
{
    return incoming() && (mChannel->callState() == Tp::CallStateInitialised);
}

QString CallEntry::phoneNumber() const
{
    if (mChannel->isConference() || !mChannel->actualFeatures().contains(Tp::CallChannel::FeatureCore) || mChannel->targetContact().isNull()) {
        return "";
    }
    return mChannel->targetContact()->id();
}

QQmlListProperty<CallEntry> CallEntry::calls()
{
    return QQmlListProperty<CallEntry>(this, 0, callsCount, callAt);
}

QQmlListProperty<AudioOutput> CallEntry::audioOutputs()
{
    return QQmlListProperty<AudioOutput>(this, 0, audioOutputsCount, audioOutputsAt);
}

bool CallEntry::isConference() const
{
    return mChannel->isConference();
}

QString CallEntry::dtmfString() const
{
    return mChannel->property("dtmfString").toString();
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

void CallEntry::splitCall()
{
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->call("SplitCall", mChannel->objectPath());
}

Tp::CallChannelPtr CallEntry::channel() const
{
    return mChannel;
}

AccountEntry *CallEntry::account() const
{
    return mAccount;
}

int CallEntry::callsCount(QQmlListProperty<CallEntry> *p)
{
    CallEntry *entry = qobject_cast<CallEntry*>(p->object);
    if (!entry) {
        return 0;
    }
    return entry->mCalls.count();
}

CallEntry *CallEntry::callAt(QQmlListProperty<CallEntry> *p, int index)
{
    CallEntry *entry = qobject_cast<CallEntry*>(p->object);
    if (!entry) {
        return 0;
    }
    return entry->mCalls[index];
}

int CallEntry::audioOutputsCount(QQmlListProperty<AudioOutput> *p)
{
    CallEntry *entry = qobject_cast<CallEntry*>(p->object);
    if (!entry) {
        return 0;
    }
    return entry->mAudioOutputs.count();
}

AudioOutput *CallEntry::audioOutputsAt(QQmlListProperty<AudioOutput> *p, int index)
{
    CallEntry *entry = qobject_cast<CallEntry*>(p->object);
    if (!entry) {
        return 0;
    }
    return entry->mAudioOutputs[index];
}

void CallEntry::addCall(CallEntry *call)
{
    mCalls << call;
    connect(call,
            SIGNAL(callEnded()),
            SLOT(onInternalCallEnded()));

    Q_EMIT callsChanged();
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

    // FIXME: maybe we should retrieve the property from the handler instead of relying on telepathy
    // for that, because on channels that are not using hardware streaming we handle the mute internally
    // with no participation of the Telepathy mute interface
    if (mChannel->handlerStreamingRequired()) {
        onMutedChanged(value ? 1 : 0);
    }
}

void CallEntry::onCallStateChanged(Tp::CallState state)
{
    qDebug() << __PRETTY_FUNCTION__ << state;
    // fetch the channel properties from the handler
    updateChannelProperties();

    switch (state) {
    case Tp::CallStateEnded:
        Q_EMIT callEnded();
        break;
    case Tp::CallStateActive:
        startTimer(1000);
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

void CallEntry::onCallLocalHoldStateChanged(Tp::LocalHoldState state, Tp::LocalHoldStateReason reason)
{
    if (reason == Tp::LocalHoldStateReasonResourceNotAvailable) {
        Q_EMIT callHoldingFailed();
    }
    Q_EMIT heldChanged();
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
    if (!mActiveTimestamp.isValid()) {
        return 0;
    }
    return mActiveTimestamp.secsTo(QDateTime::currentDateTimeUtc());
}

bool CallEntry::isActive() const
{
    return (mChannel->callState() == Tp::CallStateActive);
}

