/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Ugo Riboni <ugo.riboni@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "approverdbus.h"
#include "approveradaptor.h"

// Qt
#include <QtDBus/QDBusConnection>

static const char* DBUS_SERVICE = "com.canonical.Approver";
static const char* DBUS_OBJECT_PATH = "/com/canonical/Approver";

ApproverDBus::ApproverDBus(QObject* parent) : QObject(parent)
{
}

ApproverDBus::~ApproverDBus()
{
}

bool
ApproverDBus::connectToBus()
{
    bool ok = QDBusConnection::sessionBus().registerService(DBUS_SERVICE);
    if (!ok) {
        return false;
    }
    new TelephonyServiceApproverAdaptor(this);
    QDBusConnection::sessionBus().registerObject(DBUS_OBJECT_PATH, this);

    return true;
}

void ApproverDBus::HangUpAndAcceptCall()
{
    Q_EMIT hangUpAndAcceptCallRequested();
}

void ApproverDBus::AcceptCall()
{
    Q_EMIT acceptCallRequested();
}

void ApproverDBus::RejectCall()
{
    Q_EMIT rejectCallRequested();
}

void ApproverDBus::HandleMediaKey(bool doubleClick)
{
    Q_EMIT handleMediaKeyRequested(doubleClick);
}
