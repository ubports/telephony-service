/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Martti Piirainen <martti.piirainen@canonical.com>
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

#include "tonegenerator.h"

#include <QDebug>
#include <QTimer>
#include <QDBusMessage>
#include <QDBusConnection>

/* Use tone-generator (tonegend) via D-Bus */
#define TONEGEN_DBUS_SERVICE_NAME "com.Nokia.Telephony.Tones"
#define TONEGEN_DBUS_OBJ_PATH "/com/Nokia/Telephony/Tones"
#define TONEGEN_DBUS_IFACE_NAME TONEGEN_DBUS_SERVICE_NAME

ToneGenerator::ToneGenerator(QObject *parent) :
    QObject(parent), mPlaybackTimer(nullptr)
{
}

ToneGenerator::~ToneGenerator()
{
    if (mPlaybackTimer && mPlaybackTimer->isActive()) {
        this->stopDTMFTone();
    }
}

ToneGenerator *ToneGenerator::instance()
{
    static ToneGenerator *self = new ToneGenerator();
    return self;
}

void ToneGenerator::playDTMFTone(uint key)
{
    if (!mPlaybackTimer) {
        mPlaybackTimer = new QTimer(this);
        mPlaybackTimer->setSingleShot(true);
        connect(mPlaybackTimer, SIGNAL(timeout()), this, SLOT(stopDTMFTone()));
    }
    if (mPlaybackTimer->isActive()) {
        qDebug() << "Already playing a tone, ignore.";
        return;
    }
    if (key > 11) {
        qDebug() << "Invalid DTMF tone, ignore.";
        return;
    }
    QDBusMessage startMsg = QDBusMessage::createMethodCall(
            TONEGEN_DBUS_SERVICE_NAME,
            TONEGEN_DBUS_OBJ_PATH,
            TONEGEN_DBUS_IFACE_NAME,
            "StartEventTone" );
    QList<QVariant> toneArgs;
    toneArgs << QVariant((uint)key);
    toneArgs << QVariant((int)0);  // volume is ignored
    toneArgs << QVariant((uint)0); // duration is ignored
    startMsg.setArguments(toneArgs);
    if (QDBusConnection::sessionBus().send(startMsg)) {
        mPlaybackTimer->start(DTMF_LOCAL_PLAYBACK_DURATION);
    }
}

void ToneGenerator::stopDTMFTone()
{
    QDBusConnection::sessionBus().send(
        QDBusMessage::createMethodCall(
            TONEGEN_DBUS_SERVICE_NAME,
            TONEGEN_DBUS_OBJ_PATH,
            TONEGEN_DBUS_IFACE_NAME,
            "StopTone" ));
    if (mPlaybackTimer) {
        mPlaybackTimer->stop();
    }
}
