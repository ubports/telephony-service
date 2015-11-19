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

#include "greetercontacts.h"
#include "ringtone.h"

RingtoneWorker::RingtoneWorker(QObject *parent) :
    QObject(parent), mCallAudioPlayer(NULL), mCallAudioPlaylist(this),
    mMessageAudioPlayer(NULL)
{
    mCallAudioPlaylist.setPlaybackMode(QMediaPlaylist::Loop);
    mCallAudioPlaylist.setCurrentIndex(0);
}

void RingtoneWorker::playIncomingCallSound()
{
    if (!qgetenv("PA_DISABLED").isEmpty()) {
        return;
    }

    if (GreeterContacts::instance()->silentMode()) {
        return;
    }

    // force delete all media player instances
    stopIncomingCallSound();

    // pick up the new ringtone in case it changed in the meantime
    mCallAudioPlaylist.addMedia(QUrl::fromLocalFile(GreeterContacts::instance()->incomingCallSound()));
    mCallAudioPlayer = new QMediaPlayer(this);
    mCallAudioPlayer->setAudioRole(QMediaPlayer::AlertRole);
    mCallAudioPlayer->setPlaylist(&mCallAudioPlaylist);
    mCallAudioPlayer->play();
}

void RingtoneWorker::stopIncomingCallSound()
{
    if (mCallAudioPlayer) {
        // WORKAROUND: if we call stop and the stream is already over, qmediaplayer plays again.
        mCallAudioPlayer->pause();
        mCallAudioPlayer->deleteLater();
        mCallAudioPlayer = NULL;
    }
    mCallAudioPlaylist.clear();
}

void RingtoneWorker::playIncomingMessageSound()
{
    if (!qgetenv("PA_DISABLED").isEmpty()) {
        return;
    }

    if (GreeterContacts::instance()->silentMode()) {
        return;
    }

    if (!mMessageAudioPlayer) {
        mMessageAudioPlayer = new QMediaPlayer(this);
        mMessageAudioPlayer->setAudioRole(QMediaPlayer::AlertRole);
    }

    // WORKAROUND: there is a bug in qmediaplayer/(media-hub?) that never goes into Stopped mode.
    if (mMessageAudioPlayer->duration() == mMessageAudioPlayer->position()) {
        mMessageAudioPlayer->stop();
    }

    if (mMessageAudioPlayer->state() == QMediaPlayer::PlayingState) {
        return;
    }

    mMessageAudioPlayer->setMedia(QUrl::fromLocalFile(GreeterContacts::instance()->incomingMessageSound()));
    mMessageAudioPlayer->play();
}

void RingtoneWorker::stopIncomingMessageSound()
{
    if (mMessageAudioPlayer) {
        mMessageAudioPlayer->pause();
        mMessageAudioPlayer->deleteLater();
        mMessageAudioPlayer = NULL;
    }
}

Ringtone::Ringtone(QObject *parent) :
    QObject(parent)
{
    mWorker = new RingtoneWorker();
    mWorker->moveToThread(&mThread);
    mThread.start();
    mVibrateEffect.setDuration(500);
}

Ringtone::~Ringtone()
{
    mThread.quit();
    mThread.wait();
}

Ringtone *Ringtone::instance()
{
    static Ringtone *self = new Ringtone();
    return self;
}

void Ringtone::playIncomingCallSound()
{
    QMetaObject::invokeMethod(mWorker, "playIncomingCallSound", Qt::QueuedConnection);
}

void Ringtone::stopIncomingCallSound()
{
    QMetaObject::invokeMethod(mWorker, "stopIncomingCallSound", Qt::QueuedConnection);
}

void Ringtone::playIncomingMessageSound()
{
    if (GreeterContacts::instance()->incomingMessageVibrate()) {
        mVibrateEffect.start();
    }

    QMetaObject::invokeMethod(mWorker, "playIncomingMessageSound", Qt::QueuedConnection);
}

void Ringtone::stopIncomingMessageSound()
{
    QMetaObject::invokeMethod(mWorker, "stopIncomingMessageSound", Qt::QueuedConnection);
}
