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
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 */

#include "handlercontroller.h"

#define HANDLER_SERVICE "com.canonical.TelephonyServiceHandler"
#define HANDLER_OBJECT "/com/canonical/TelephonyServiceHandler"
#define HANDLER_INTERFACE "com.canonical.TelephonyServiceHandler"

HandlerController *HandlerController::instance()
{
    static HandlerController *self = new HandlerController();
    return self;
}

HandlerController::HandlerController(QObject *parent) :
    QObject(parent),
    mHandlerInterface(HANDLER_SERVICE, HANDLER_OBJECT, HANDLER_INTERFACE)
{
}

void HandlerController::startCall(const QString &number)
{
    mHandlerInterface.call("StartCall", number);
}

void HandlerController::hangUpCall(const QString &objectPath)
{
    mHandlerInterface.call("HangUpCall", objectPath);
}

void HandlerController::setHold(const QString &objectPath, bool hold)
{
    mHandlerInterface.call("SetHold", objectPath, hold);
}

void HandlerController::setMuted(const QString &objectPath, bool muted)
{
    mHandlerInterface.call("SetMuted", objectPath, muted);
}

void HandlerController::setSpeakerMode(const QString &objectPath, bool enabled)
{
    mHandlerInterface.call("SetSpeakerMode", objectPath, enabled);
}

void HandlerController::sendDTMF(const QString &objectPath, const QString &key)
{
    mHandlerInterface.call("SendDTMF", objectPath, key);
}

void HandlerController::sendMessage(const QString &number, const QString &message)
{
    mHandlerInterface.call("SendMessage", number, message);
}

void HandlerController::acknowledgeMessages(const QString &number, const QStringList &messageIds)
{
    mHandlerInterface.call("AcknowledgeMessages", number, messageIds);
}
