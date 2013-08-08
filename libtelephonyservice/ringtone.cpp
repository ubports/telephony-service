/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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

#include "ringtone.h"
#include <QDebug>

Ringtone::Ringtone(QObject *parent) :
    QObject(parent),
    mCallAudioPlayer(this), mCallAudioPlaylist(this), mMessageAudioPlayer(this), mSoundSettings("com.ubuntu.touch.sound")
{
    mCallAudioPlaylist.addMedia(QUrl::fromLocalFile(mSoundSettings.get("incomingCallSound").toString()));
    mCallAudioPlaylist.setPlaybackMode(QMediaPlaylist::Loop);
    mCallAudioPlaylist.setCurrentIndex(0);
}


Ringtone *Ringtone::instance()
{
    static Ringtone *self = new Ringtone();
    return self;
}

void Ringtone::playIncomingCallSound()
{
    if (mSoundSettings.get("silentMode") == true) {
        return;
    }

    if (mCallAudioPlayer.state() == QMediaPlayer::PlayingState) {
        return;
    }

    mCallAudioPlayer.setPlaylist(&mCallAudioPlaylist);
    mCallAudioPlayer.play();
}

void Ringtone::stopIncomingCallSound()
{
    mCallAudioPlayer.stop();
}

void Ringtone::playIncomingMessageSound()
{
    if (mSoundSettings.get("silentMode") == true) {
        return;
    }

    if (mMessageAudioPlayer.state() == QMediaPlayer::PlayingState) {
        return;
    }

    mMessageAudioPlayer.setMedia(QUrl::fromLocalFile(mSoundSettings.get("incomingMessageSound").toString()));
    mMessageAudioPlayer.play();
}

void Ringtone::stopIncomingMessageSound()
{
    mMessageAudioPlayer.stop();
}

