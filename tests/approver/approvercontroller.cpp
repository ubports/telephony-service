/**
 * Copyright (C) 2013-2015 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 */

#include <QStringList>
#include "approvercontroller.h"
#include <QDBusReply>
#include <QDebug>

#define APPROVER_SERVICE "com.canonical.Approver"
#define APPROVER_OBJECT "/com/canonical/Approver"
#define APPROVER_INTERFACE "com.canonical.TelephonyServiceApprover"

ApproverController *ApproverController::instance()
{
    static ApproverController *self = new ApproverController();
    return self;
}

ApproverController::ApproverController(QObject *parent) :
    QObject(parent),
    mApproverInterface(APPROVER_SERVICE, APPROVER_OBJECT, APPROVER_INTERFACE)
{
}

void ApproverController::acceptCall()
{
    mApproverInterface.call("AcceptCall");
}

void ApproverController::hangUpAndAcceptCall()
{
    mApproverInterface.call("HangUpAndAcceptCall");
}
