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

#ifndef AUDIOROUTEMANAGER_H
#define AUDIOROUTEMANAGER_H

#ifdef USE_PULSEAUDIO
#include "qpulseaudioengine.h"
#endif
#include "audiooutput.h"
#include "powerdaudiomodemediator.h"
#include "powerddbus.h"
#include <QObject>
#include <TelepathyQt/CallChannel>


class AudioRouteManager : public QObject
{
    Q_OBJECT

public:
    static AudioRouteManager *instance();
    void setActiveAudioOutput(const QString &id);
    QString activeAudioOutput();
    AudioOutputDBusList audioOutputs() const;
    void updateAudioRoute(bool newCall = false);

public Q_SLOTS:
    void onCallChannelAvailable(Tp::CallChannelPtr callChannel);

Q_SIGNALS:
    void audioOutputsChanged(const AudioOutputDBusList &outputs);
    void activeAudioOutputChanged(const QString &id);
    void lastChannelClosed();

protected Q_SLOTS:
    void onCallStateChanged(Tp::CallState state);

private Q_SLOTS:
#ifdef USE_PULSEAUDIO
    void onAudioModeChanged(AudioMode mode);
    void onAvailableAudioModesChanged(AudioModes modes);
#endif

private:
    explicit AudioRouteManager(QObject *parent = 0);
    QList<Tp::CallChannelPtr> mChannels;
    AudioOutputDBusList mAudioOutputs;
    QString mActiveAudioOutput;
    PowerDDBus mPowerDDBus;
    PowerDAudioModeMediator mAudioModeMediator;
#ifdef USE_PULSEAUDIO
    bool mHasPulseAudio;
#endif
};

#endif // AUDIOROUTEMANAGER_H
