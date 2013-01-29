/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ringtone.h"
#include <QDebug>

#define SOUND_PATH "/usr/share/sounds/ubuntu/stereo/"
Ringtone::Ringtone(QObject *parent) :
    QObject(parent),
    mCallAudioFile(SOUND_PATH "phone-incoming-call.ogg"),
    mMessageAudioFile(SOUND_PATH "message-new-instant.ogg"),
    mCallAudioPlaying(false), mMessageAudioPlaying(false)
{
    connect(&mCallAudioOutput,
            SIGNAL(stateChanged(QAudio::State)),
            SLOT(onCallAudioStateChanged(QAudio::State)));
    connect(&mMessageAudioOutput,
            SIGNAL(stateChanged(QAudio::State)),
            SLOT(onMessageAudioStateChanged(QAudio::State)));
}


Ringtone *Ringtone::instance()
{
    static Ringtone *self = new Ringtone();
    return self;
}

void Ringtone::playIncomingCallSound()
{
    if (mCallAudioPlaying) {
        return;
    }

    mCallAudioPlaying = true;
    if (mCallAudioFile.open(QIODevice::ReadOnly)) {
        mCallAudioOutput.start(&mCallAudioFile);
    } else {
        qCritical() << "Unable to open sound file" << mCallAudioFile.fileName();
        qCritical() << "Error:" << mCallAudioFile.errorString();
    }
}

void Ringtone::stopIncomingCallSound()
{
    if (!mCallAudioPlaying) {
        return;
    }

    mCallAudioPlaying = false;
    mCallAudioOutput.stop();
}

void Ringtone::playIncomingMessageSound()
{
    if (mMessageAudioPlaying) {
        return;
    }

    mMessageAudioPlaying = true;
    if (mMessageAudioFile.open(QIODevice::ReadOnly)) {
        mMessageAudioOutput.start((&mMessageAudioFile));
    } else {
        qCritical() << "Unable to open sound file" << mMessageAudioFile.fileName();
        qCritical() << "Error:" << mMessageAudioFile.errorString();
    }
}

void Ringtone::stopIncomingMessageSound()
{
    if (!mMessageAudioPlaying) {
        return;
    }

    mMessageAudioOutput.stop();
}

void Ringtone::onCallAudioStateChanged(QAudio::State state)
{
    if (state != QAudio::StoppedState) {
        return;
    }

    if (mCallAudioPlaying) {
        // loop playing the incoming call sound
        mCallAudioOutput.start(&mCallAudioFile);
    } else {
        // or close it in case we stopped ringing
        mCallAudioFile.close();
    }
}

void Ringtone::onMessageAudioStateChanged(QAudio::State state)
{
    if (state != QAudio::StoppedState) {
        return;
    }

    mMessageAudioFile.close();
    mMessageAudioPlaying = false;
}
