/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Ugo Riboni <ugo.riboni@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "phoneappapproverdbus.h"
#include "phoneappapproveradaptor.h"

// Qt
#include <QtDBus/QDBusConnection>

static const char* DBUS_SERVICE = "com.canonical.PhoneAppApprover";
static const char* DBUS_OBJECT_PATH = "/com/canonical/PhoneAppApprover";

PhoneAppApproverDBus::PhoneAppApproverDBus(QObject* parent) : QObject(parent)
{
}

PhoneAppApproverDBus::~PhoneAppApproverDBus()
{
}

bool
PhoneAppApproverDBus::connectToBus()
{
    bool ok = QDBusConnection::sessionBus().registerService(DBUS_SERVICE);
    if (!ok) {
        return false;
    }
    new PhoneAppApproverAdaptor(this);
    QDBusConnection::sessionBus().registerObject(DBUS_OBJECT_PATH, this);

    return true;
}

void PhoneAppApproverDBus::SendMessage(const QString &number, const QString &message)
{
    Q_EMIT onMessageSent(number, message);
}

void PhoneAppApproverDBus::AcceptCall()
{
    Q_EMIT acceptCallRequested();
}

void PhoneAppApproverDBus::RejectCall()
{
    Q_EMIT rejectCallRequested();
}
