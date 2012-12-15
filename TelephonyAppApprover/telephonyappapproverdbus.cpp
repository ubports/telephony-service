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

#include "telephonyappapproverdbus.h"
//#include "telephonyappadaptor.h"
#include "TelephonyAppApprover/telephonyappapproveradaptor.h"

// Qt
#include <QtDBus/QDBusConnection>

static const char* DBUS_SERVICE = "com.canonical.TelephonyAppApprover";
static const char* DBUS_OBJECT_PATH = "/com/canonical/TelephonyAppApprover";

TelephonyAppApproverDBus::TelephonyAppApproverDBus(QObject* parent) : QObject(parent)
{
}

TelephonyAppApproverDBus::~TelephonyAppApproverDBus()
{
}

bool
TelephonyAppApproverDBus::connectToBus()
{
    bool ok = QDBusConnection::sessionBus().registerService(DBUS_SERVICE);
    if (!ok) {
        return false;
    }
    new TelephonyAppApproverAdaptor(this);
    QDBusConnection::sessionBus().registerObject(DBUS_OBJECT_PATH, this);

    return true;
}

void TelephonyAppApproverDBus::SendMessage(const QString &number, const QString &message)
{
    Q_EMIT onMessageSent(number, message);
}
