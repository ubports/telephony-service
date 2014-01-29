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

#define AS_SERVICE "org.freedesktop.Accounts"
#define AS_PATH "/org/freedesktop/Accounts"
#define AS_IFACE "org.freedesktop.Accounts"
#define AS_EXTENSION_IFACE "com.ubuntu.touch.AccountsService.Sound"

RingtoneWorker::RingtoneWorker(QObject *parent) :
    QObject(parent), mCallAudioPlayer(this), mCallAudioPlaylist(this),
    mMessageAudioPlayer(this),
    mSystemBusConnection(QDBusConnection::systemBus()),
    mServiceWatcher(AS_SERVICE,
                     mSystemBusConnection,
                     QDBusServiceWatcher::WatchForOwnerChange),
    mAccountsserviceIface(AS_SERVICE,
                           AS_PATH,
                           AS_IFACE,
                           mSystemBusConnection)
{
    mCallAudioPlaylist.setPlaybackMode(QMediaPlaylist::Loop);
    mCallAudioPlaylist.setCurrentIndex(0);

    connect (&mServiceWatcher,
             SIGNAL (serviceOwnerChanged (QString, QString, QString)),
             this,
             SLOT (onNameOwnerChanged (QString, QString, QString)));

    if (mAccountsserviceIface.isValid()) {
        setUpInterface();
    }
}

void RingtoneWorker::onNameOwnerChanged(QString name,
                                        QString oldOwner,
                                        QString newOwner)
{
    Q_UNUSED (oldOwner);
    Q_UNUSED (newOwner);
    if (name != "org.freedesktop.Accounts")
        return;

    setUpInterface();
}

void RingtoneWorker::onChanged(QString interface,
                               QVariantMap changedProperties,
                               QStringList invalidatedProperties)
{
    Q_UNUSED (interface);
    Q_UNUSED (invalidatedProperties);

    if (changedProperties.contains("SilentMode"))
        mSilentMode = getUserProperty("SilentMode").toBool();

    if (changedProperties.contains("IncomingCallSound"))
        mIncomingCallSound = getUserProperty("IncomingCallSound").toString();

    if (changedProperties.contains("IncomingMessageSound"))
        mIncomingMessageSound = getUserProperty("IncomingMessageSound").toString();
}

void RingtoneWorker::setUpInterface()
{
    /* Here we should figure out which is the active user and get that user's
     * AS object path. */
    QDBusReply<QDBusObjectPath> qObjectPath = mAccountsserviceIface.call(
                "FindUserById", qlonglong(getuid()));

    if (qObjectPath.isValid()) {
        mObjectPath = qObjectPath.value().path();
        mAccountsserviceIface.connection().connect(
            mAccountsserviceIface.service(),
            mObjectPath,
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged",
            this,
            SLOT(onChanged(QString, QVariantMap, QStringList)));

        mSilentMode = getUserProperty("SilentMode").toBool();
        mIncomingCallSound = getUserProperty("IncomingCallSound").toString();
        mIncomingMessageSound = getUserProperty("IncomingMessageSound").toString();
    }
}

QVariant RingtoneWorker::getUserProperty(const QString property)
{
    if (!mAccountsserviceIface.isValid())
        return QVariant();

    QDBusInterface iface (
                "org.freedesktop.Accounts",
                mObjectPath,
                "org.freedesktop.DBus.Properties",
                mSystemBusConnection,
                this);

    if (iface.isValid()) {
        QDBusReply<QDBusVariant> answer = iface.call(
                    "Get",
                    AS_EXTENSION_IFACE,
                    property);
        if (answer.isValid()) {
            return answer.value().variant();
        }
    }
    return QVariant();
}

void RingtoneWorker::playIncomingCallSound()
{
    if (mSilentMode) {
        return;
    }

    if (mCallAudioPlayer.state() == QMediaPlayer::PlayingState) {
        return;
    }

    mCallAudioPlaylist.clear();
    mCallAudioPlaylist.addMedia(QUrl::fromLocalFile(mIncomingCallSound));
    mCallAudioPlayer.setPlaylist(&mCallAudioPlaylist);
    mCallAudioPlayer.play();
}

void RingtoneWorker::stopIncomingCallSound()
{
    mCallAudioPlayer.stop();
}

void RingtoneWorker::playIncomingMessageSound()
{
    if (mSilentMode) {
        return;
    }

    if (mMessageAudioPlayer.state() == QMediaPlayer::PlayingState) {
        return;
    }

    mMessageAudioPlayer.setMedia(QUrl::fromLocalFile(mIncomingMessageSound));
    mMessageAudioPlayer.play();
}

void RingtoneWorker::stopIncomingMessageSound()
{
    mMessageAudioPlayer.stop();
}

Ringtone::Ringtone(QObject *parent) :
    QObject(parent)
{
    mWorker = new RingtoneWorker(this);
    mWorker->moveToThread(&mThread);
    mThread.start();
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
    QMetaObject::invokeMethod(mWorker, "playIncomingMessageSound", Qt::QueuedConnection);
}

void Ringtone::stopIncomingMessageSound()
{
    QMetaObject::invokeMethod(mWorker, "stopIncomingMessageSound", Qt::QueuedConnection);
}
