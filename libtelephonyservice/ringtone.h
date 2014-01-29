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

#ifndef RINGTONE_H
#define RINGTONE_H

#include <QObject>
#include <QThread>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QFile>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QtDBus/QDBusInterface>
#include <unistd.h>
#include <sys/types.h>


class RingtoneWorker : public QObject
{
    Q_OBJECT
public:
    RingtoneWorker(QObject *parent = 0);

public Q_SLOTS:
    void playIncomingCallSound();
    void stopIncomingCallSound();
    void playIncomingMessageSound();
    void stopIncomingMessageSound();


private Q_SLOTS:
    void onChanged(QString, QVariantMap, QStringList);
    void onNameOwnerChanged(QString, QString, QString);

private:
    QMediaPlayer mCallAudioPlayer;
    QMediaPlaylist mCallAudioPlaylist;

    QMediaPlayer mMessageAudioPlayer;

    QDBusConnection mSystemBusConnection;
    QDBusServiceWatcher mServiceWatcher;
    QDBusInterface mAccountsserviceIface;
    QString mObjectPath;

    bool mSilentMode;
    QString mIncomingCallSound;
    QString mIncomingMessageSound;

    void setUpInterface();
    QVariant getUserProperty(const QString property);
};

class Ringtone : public QObject
{
    Q_OBJECT
public:
    ~Ringtone();
    static Ringtone *instance();

public Q_SLOTS:
    void playIncomingCallSound();
    void stopIncomingCallSound();

    void playIncomingMessageSound();
    void stopIncomingMessageSound();

private:
    explicit Ringtone(QObject *parent = 0);
    RingtoneWorker *mWorker;
    QThread mThread;
};

#endif // RINGTONE_H
