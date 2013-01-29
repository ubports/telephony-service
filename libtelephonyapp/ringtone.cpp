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
    mCallAudioPlayer(this), mCallAudioPlaylist(this), mMessageAudioPlayer(this),
    mCallAudioPlaying(false), mMessageAudioPlaying(false)
{
    mCallAudioPlaylist.addMedia(QUrl::fromLocalFile(SOUND_PATH "phone-incoming-call.ogg"));
    mCallAudioPlaylist.setPlaybackMode(QMediaPlaylist::Loop);
    mCallAudioPlaylist.setCurrentIndex(0);
    connect(&mCallAudioPlayer,
            SIGNAL(stateChanged(QMediaPlayer::State)),
            SLOT(onCallAudioStateChanged(QMediaPlayer::State)));

    connect(&mMessageAudioPlayer,
            SIGNAL(stateChanged(QMediaPlayer::State)),
            SLOT(onMessageAudioStateChanged(QMediaPlayer::State)));
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
    mCallAudioPlayer.setPlaylist(&mCallAudioPlaylist);
    mCallAudioPlayer.play();
}

void Ringtone::stopIncomingCallSound()
{
    if (!mCallAudioPlaying) {
        return;
    }

    mCallAudioPlayer.stop();
}

void Ringtone::playIncomingMessageSound()
{
    if (mMessageAudioPlaying) {
        return;
    }

    mMessageAudioPlaying = true;
    mMessageAudioPlayer.setMedia(QUrl::fromLocalFile(SOUND_PATH "message-new-instant.ogg"));
    mMessageAudioPlayer.play();
}

void Ringtone::stopIncomingMessageSound()
{
    if (!mMessageAudioPlaying) {
        return;
    }

    mMessageAudioPlayer.stop();
}

void Ringtone::onCallAudioStateChanged(QMediaPlayer::State state)
{
    if (state != QMediaPlayer::StoppedState) {
        return;
    }

    mCallAudioPlaying = false;
}

void Ringtone::onMessageAudioStateChanged(QMediaPlayer::State state)
{
    if (state != QMediaPlayer::StoppedState) {
        return;
    }

    mMessageAudioPlaying = false;
}
