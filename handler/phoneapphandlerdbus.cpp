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

#include "callhandler.h"
#include "phoneapphandlerdbus.h"
#include "phoneapphandleradaptor.h"
#include "texthandler.h"

// Qt
#include <QtDBus/QDBusConnection>

static const char* DBUS_SERVICE = "com.canonical.PhoneAppHandler";
static const char* DBUS_OBJECT_PATH = "/com/canonical/PhoneAppHandler";

PhoneAppHandlerDBus::PhoneAppHandlerDBus(QObject* parent) : QObject(parent)
{
}

PhoneAppHandlerDBus::~PhoneAppHandlerDBus()
{
}

bool PhoneAppHandlerDBus::connectToBus()
{
    bool ok = QDBusConnection::sessionBus().registerService(DBUS_SERVICE);
    if (!ok) {
        return false;
    }
    new PhoneAppHandlerAdaptor(this);
    QDBusConnection::sessionBus().registerObject(DBUS_OBJECT_PATH, this);

    return true;
}

void PhoneAppHandlerDBus::SendMessage(const QString &number, const QString &message)
{
    TextHandler::instance()->sendMessage(number, message);
}

void PhoneAppHandlerDBus::AcknowledgeMessages(const QString &number, const QStringList &messageIds)
{
    TextHandler::instance()->acknowledgeMessages(number, messageIds);
}

void PhoneAppHandlerDBus::StartCall(const QString &number)
{
    CallHandler::instance()->startCall(number);
}

void PhoneAppHandlerDBus::HangUpCall(const QString &objectPath)
{
    CallHandler::instance()->hangUpCall(objectPath);
}

void PhoneAppHandlerDBus::SetHold(const QString &objectPath, bool hold)
{
    CallHandler::instance()->setHold(objectPath, hold);
}

void PhoneAppHandlerDBus::SetMuted(const QString &objectPath, bool muted)
{
    CallHandler::instance()->setMuted(objectPath, muted);
}

void PhoneAppHandlerDBus::SetSpeakerMode(const QString &objectPath, bool enabled)
{
    CallHandler::instance()->setSpeakerMode(objectPath, enabled);
}

void PhoneAppHandlerDBus::SendDTMF(const QString &objectPath, const QString &key)
{
    CallHandler::instance()->sendDTMF(objectPath, key);
}
