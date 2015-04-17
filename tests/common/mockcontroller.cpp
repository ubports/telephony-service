/**
 * Copyright (C) 2013 Canonical, Ltd.
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
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 */

#include "mockcontroller.h"
#include <QDBusReply>

static const QString mockService("com.canonical.MockConnection");
static const QString mockObject("/com/canonical/MockConnection/%1");
static const QString mockInterface("com.canonical.MockConnection");

MockController::MockController(const QString &protocol, QObject *parent) :
    ComCanonicalMockConnectionInterface(mockService, mockObject.arg(protocol), QDBusConnection::sessionBus(), this),
    mProtocol(protocol), mMockObject(mockObject.arg(protocol))
{
}

QString MockController::placeCall(const QVariantMap &properties)
{
    QDBusPendingReply<QString> reply = PlaceCall(properties);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return QString::null;
    }
    return reply.value();
}

QString MockController::serial()
{
    QDBusPendingReply<QString> reply = Serial();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return QString::null;
    }

    return reply.value();
}
