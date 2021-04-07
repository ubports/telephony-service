/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * Authors:
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

#include "audioroutemanager.h"
#include "telepathyhelper.h"
#include "accountentry.h"
#include <TelepathyQt/Contact>
#include <TelepathyQt/Functors>


static void enable_earpiece()
{
#ifdef USE_PULSEAUDIO
    QPulseAudioEngine::instance()->setCallMode(CallActive, AudioModeBtOrWiredOrEarpiece);
#endif
}

static void enable_normal()
{
#ifdef USE_PULSEAUDIO
    QTimer* timer = new QTimer();
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, [=](){
        QPulseAudioEngine::instance()->setMicMute(false);
        QPulseAudioEngine::instance()->setCallMode(CallEnded, AudioModeWiredOrSpeaker);
        timer->deleteLater();
    });
    timer->start(2200);
#endif
}

static void enable_speaker()
{
#ifdef USE_PULSEAUDIO
    QPulseAudioEngine::instance()->setCallMode(CallActive, AudioModeSpeaker);
#endif
}

static void enable_ringtone()
{
#ifdef USE_PULSEAUDIO
    QPulseAudioEngine::instance()->setCallMode(CallRinging, AudioModeBtOrWiredOrSpeaker);
#endif
}

AudioRouteManager *AudioRouteManager::instance()
{
    static AudioRouteManager *self = new AudioRouteManager();
    return self;
}

AudioRouteManager::AudioRouteManager(QObject *parent) :
    QObject(parent), mAudioModeMediator(mPowerDDBus)
{
    TelepathyHelper::instance()->registerChannelObserver("TelephonyServiceHandlerAudioRouteManager");

    QObject::connect(TelepathyHelper::instance()->channelObserver(), SIGNAL(callChannelAvailable(Tp::CallChannelPtr)),
                 this, SLOT(onCallChannelAvailable(Tp::CallChannelPtr)));

#ifdef USE_PULSEAUDIO
    // update audio modes
    QObject::connect(QPulseAudioEngine::instance(), SIGNAL(audioModeChanged(AudioMode)), SLOT(onAudioModeChanged(AudioMode)));
    QObject::connect(QPulseAudioEngine::instance(), SIGNAL(availableAudioModesChanged(AudioModes)), SLOT(onAvailableAudioModesChanged(AudioModes)));

    // check if we should indeed use pulseaudio
    QByteArray pulseAudioDisabled = qgetenv("PA_DISABLED");
    mHasPulseAudio = true;
    if (!pulseAudioDisabled.isEmpty())
        mHasPulseAudio = false;
#endif

    connect(this, &AudioRouteManager::activeAudioOutputChanged, Tp::memFun(&mAudioModeMediator, &PowerDAudioModeMediator::audioModeChanged));
    connect(this, &AudioRouteManager::lastChannelClosed, Tp::memFun(&mAudioModeMediator, &PowerDAudioModeMediator::audioOutputClosed));
}

void AudioRouteManager::onCallChannelAvailable(Tp::CallChannelPtr callChannel)
{
    connect(callChannel.data(),
                SIGNAL(callStateChanged(Tp::CallState)),
                SLOT(onCallStateChanged(Tp::CallState)));

    mChannels.append(callChannel);
    updateAudioRoute(true);
}

void AudioRouteManager::onCallStateChanged(Tp::CallState state)
{
    Tp::CallChannelPtr channel(qobject_cast<Tp::CallChannel*>(sender()));
    if (!channel) {
        return;
    }

    if (channel->callState() ==  Tp::CallStateEnded) {
        mChannels.removeOne(channel);
    }
    updateAudioRoute(false);
}

void AudioRouteManager::setActiveAudioOutput(const QString &id)
{
#ifdef USE_PULSEAUDIO
    // fallback to earpiece/headset
    AudioMode mode = AudioModeWiredOrEarpiece;
    if (id == "bluetooth") {
        mode = AudioModeBluetooth;
    } else if (id == "speaker") {
        mode = AudioModeSpeaker;
    }
    if (mHasPulseAudio)
        QPulseAudioEngine::instance()->setCallMode(CallActive, mode);
#endif
}

QString AudioRouteManager::activeAudioOutput()
{
    return mActiveAudioOutput;
}

AudioOutputDBusList AudioRouteManager::audioOutputs() const
{
    return mAudioOutputs;
}

void AudioRouteManager::updateAudioRoute(bool newCall)
{
#ifdef USE_PULSEAUDIO
    if (!mHasPulseAudio)
        return;
#endif

    int currentCalls = mChannels.size();
    if (currentCalls != 0) {
        if (currentCalls == 1) {
            // if we have only one call, check if it's incoming and
            // enable speaker mode so the ringtone is audible
            Tp::CallChannelPtr callChannel = mChannels.first();
            AccountEntry *accountEntry = TelepathyHelper::instance()->accountForConnection(callChannel->connection());
            if (!accountEntry || !callChannel) {
                return;
            }

            bool incoming = callChannel->initiatorContact() != accountEntry->account()->connection()->selfContact();
            Tp::CallState state = callChannel->callState();
            if (incoming && newCall) {
                enable_ringtone();
                return;
            }
            if (state == Tp::CallStateEnded) {
                enable_normal();
                return;
            }
            // if only one call and dialing, or incoming call just accepted, then default to earpiece
            if (newCall || (state == Tp::CallStateAccepted && incoming)) {
                enable_earpiece();
                return;
            }
        }
    } else {
        enable_normal();
        Q_EMIT lastChannelClosed();
    }
}

#ifdef USE_PULSEAUDIO
void AudioRouteManager::onAudioModeChanged(AudioMode mode)
{
    qDebug("PulseAudio audio mode changed: 0x%x", mode);

    if (mode == AudioModeEarpiece && mActiveAudioOutput != "earpiece") {
        mActiveAudioOutput = "earpiece";
    } else if (mode == AudioModeWiredHeadset && mActiveAudioOutput != "wired_headset") {
        mActiveAudioOutput = "wired_headset";
    } else if (mode == AudioModeSpeaker && mActiveAudioOutput != "speaker") {
        mActiveAudioOutput = "speaker";
    } else if (mode == AudioModeBluetooth && mActiveAudioOutput != "bluetooth") {
        mActiveAudioOutput = "bluetooth";
    }
    Q_EMIT activeAudioOutputChanged(mActiveAudioOutput);
}

void AudioRouteManager::onAvailableAudioModesChanged(AudioModes modes)
{
    qDebug("PulseAudio available audio modes changed");
    bool defaultFound = false;
    mAudioOutputs.clear();
    Q_FOREACH(const AudioMode &mode, modes) {
        AudioOutputDBus output;
        if (mode == AudioModeBluetooth) {
            // there can be only one bluetooth
            output.id = "bluetooth";
            output.type = "bluetooth";
            // we dont support names for now, so we set a default value
            output.name = "bluetooth";
        } else if (mode == AudioModeEarpiece || mode == AudioModeWiredHeadset) {
            if (!defaultFound) {
                defaultFound = true;
                output.id = "default";
                output.type = "default";
                output.name = "default";
            } else {
                continue;
            }
        } else if (mode == AudioModeSpeaker) {
            output.id = "speaker";
            output.type = "speaker";
            output.name = "speaker";
        }
        mAudioOutputs << output;
    }
    Q_EMIT audioOutputsChanged(mAudioOutputs);
}
#endif

