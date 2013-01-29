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
    mCallAudioPlayer(this), mMessageAudioPlayer(this)
{
    mCallAudioPlayer.setSource(QUrl::fromLocalFile(SOUND_PATH "phone-incoming-call.ogg"));
    mCallAudioPlayer.setLoopCount(QSoundEffect::Infinite);

    mMessageAudioPlayer.setSource(QUrl::fromLocalFile(SOUND_PATH "message-new-instant.ogg"));
}


Ringtone *Ringtone::instance()
{
    static Ringtone *self = new Ringtone();
    return self;
}

void Ringtone::playIncomingCallSound()
{
    if (mCallAudioPlayer.isPlaying()) {
        return;
    }

    mCallAudioPlayer.play();
}

void Ringtone::stopIncomingCallSound()
{
    if (mCallAudioPlayer.isPlaying()) {
        mCallAudioPlayer.stop();
    }
}

void Ringtone::playIncomingMessageSound()
{
    if (mMessageAudioPlayer.isPlaying()) {
        return;
    }

    mMessageAudioPlayer.play();
}

void Ringtone::stopIncomingMessageSound()
{
    if (mMessageAudioPlayer.isPlaying()) {
        mMessageAudioPlayer.stop();
    }
}
