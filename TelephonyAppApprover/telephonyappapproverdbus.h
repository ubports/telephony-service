/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Ugo Riboni <ugo.riboni@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TELEPHONYAPPROVERDBUS_H
#define TELEPHONYAPPROVERDBUS_H

#include <QtCore/QObject>
#include <QtDBus/QDBusContext>
#include "chatmanager.h"

/**
 * DBus interface for the telephony approver
 */
class TelephonyAppApproverDBus : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    TelephonyAppApproverDBus(QObject* parent=0);
    ~TelephonyAppApproverDBus();

    bool connectToBus();

public Q_SLOTS:
    Q_NOREPLY void SendMessage(const QString &number, const QString &message);

Q_SIGNALS:
    void onMessageSent(const QString &number, const QString &message);
};

#endif // TELEPHONYAPPROVERDBUS_H
