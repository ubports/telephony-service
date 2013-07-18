/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Ugo Riboni <ugo.riboni@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "callhandler.h"
#include "handlerdbus.h"
#include "handleradaptor.h"
#include "texthandler.h"

// Qt
#include <QtDBus/QDBusConnection>

static const char* DBUS_SERVICE = "com.canonical.TelephonyServiceHandler";
static const char* DBUS_OBJECT_PATH = "/com/canonical/TelephonyServiceHandler";

HandlerDBus::HandlerDBus(QObject* parent) : QObject(parent)
{
}

HandlerDBus::~HandlerDBus()
{
}

bool HandlerDBus::connectToBus()
{
    bool ok = QDBusConnection::sessionBus().registerService(DBUS_SERVICE);
    if (!ok) {
        return false;
    }
    new TelephonyServiceHandlerAdaptor(this);
    QDBusConnection::sessionBus().registerObject(DBUS_OBJECT_PATH, this);

    return true;
}

void HandlerDBus::SendMessage(const QString &number, const QString &message)
{
    TextHandler::instance()->sendMessage(number, message);
}

void HandlerDBus::AcknowledgeMessages(const QString &number, const QStringList &messageIds)
{
    TextHandler::instance()->acknowledgeMessages(number, messageIds);
}

void HandlerDBus::StartCall(const QString &number)
{
    CallHandler::instance()->startCall(number);
}

void HandlerDBus::HangUpCall(const QString &objectPath)
{
    CallHandler::instance()->hangUpCall(objectPath);
}

void HandlerDBus::SetHold(const QString &objectPath, bool hold)
{
    CallHandler::instance()->setHold(objectPath, hold);
}

void HandlerDBus::SetMuted(const QString &objectPath, bool muted)
{
    CallHandler::instance()->setMuted(objectPath, muted);
}

void HandlerDBus::SetSpeakerMode(const QString &objectPath, bool enabled)
{
    CallHandler::instance()->setSpeakerMode(objectPath, enabled);
}

void HandlerDBus::SendDTMF(const QString &objectPath, const QString &key)
{
    CallHandler::instance()->sendDTMF(objectPath, key);
}
