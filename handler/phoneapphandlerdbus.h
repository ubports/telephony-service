/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Ugo Riboni <ugo.riboni@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PHONEAPPHANDLERDBUS_H
#define PHONEAPPHANDLERDBUS_H

#include <QtCore/QObject>
#include <QtDBus/QDBusContext>
#include "chatmanager.h"

/**
 * DBus interface for the phone approver
 */
class PhoneAppHandlerDBus : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    PhoneAppHandlerDBus(QObject* parent=0);
    ~PhoneAppHandlerDBus();

public Q_SLOTS:
    bool connectToBus();

    // messages related
    Q_NOREPLY void SendMessage(const QString &number, const QString &message);
    Q_NOREPLY void AcknowledgeMessages(const QString &number, const QStringList &messageIds);

    // call related
    Q_NOREPLY void StartCall(const QString &number);
    Q_NOREPLY void HangUpCall(const QString &objectPath);
    Q_NOREPLY void SetHold(const QString &objectPath, bool hold);
    Q_NOREPLY void SetMuted(const QString &objectPath, bool muted);
    Q_NOREPLY void SetSpeakerMode(const QString &objectPath, bool enabled);
    Q_NOREPLY void SendDTMF(const QString &objectPath, const QString &key);

Q_SIGNALS:
    void onMessageSent(const QString &number, const QString &message);
};

#endif // PHONEAPPROVERDBUS_H
