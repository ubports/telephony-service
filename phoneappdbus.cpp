/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Ugo Riboni <ugo.riboni@canonical.com>
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

#include "phoneappdbus.h"
#include "phoneappadaptor.h"

// Qt
#include <QtDBus/QDBusConnection>

static const char* DBUS_SERVICE = "com.canonical.PhoneApp";
static const char* DBUS_OBJECT_PATH = "/com/canonical/PhoneApp";

PhoneAppDBus::PhoneAppDBus(QObject* parent) : QObject(parent)
{
}

PhoneAppDBus::~PhoneAppDBus()
{
}

bool
PhoneAppDBus::connectToBus()
{
    bool ok = QDBusConnection::sessionBus().registerService(DBUS_SERVICE);
    if (!ok) {
        return false;
    }
    new PhoneAppAdaptor(this);
    QDBusConnection::sessionBus().registerObject(DBUS_OBJECT_PATH, this);

    return true;
}

void
PhoneAppDBus::ShowMessages(const QString &number)
{
    Q_EMIT request(QString("message://%1").arg(number));
}

void PhoneAppDBus::ShowMessage(const QString &messageId)
{
    Q_EMIT request(QString("messageId://%1").arg(messageId));
}

void PhoneAppDBus::NewMessage()
{
    Q_EMIT request(QString("message://"));
}

void PhoneAppDBus::SendMessage(const QString &number, const QString &message)
{
    Q_EMIT messageSendRequested(number, message);
}

void PhoneAppDBus::ShowVoicemail()
{
    Q_EMIT request(QString("voicemail://"));
}

void
PhoneAppDBus::CallNumber(const QString &number)
{
    Q_EMIT request(QString("call://%1").arg(number));
}

void PhoneAppDBus::SendAppMessage(const QString &message)
{
    Q_EMIT request(message);
}
