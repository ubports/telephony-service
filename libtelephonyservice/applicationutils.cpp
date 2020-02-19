/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
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

#include "applicationutils.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QProcess>
#include <QDesktopServices>
#include <TelepathyQt/Constants>

ApplicationUtils::ApplicationUtils(QObject *parent) :
    QObject(parent)
{
}

ApplicationUtils *ApplicationUtils::instance()
{
    static ApplicationUtils *self = new ApplicationUtils();
    return self;
}

bool ApplicationUtils::checkApplicationRunning(const QString &serviceName)
{
    bool result = false;
    QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered(serviceName);
    if (reply.isValid()) {
        result = reply.value();
    }

    return result;
}


#ifdef WANT_UI_SERVICES

#include <url-dispatcher.h>

static void
urlDispatchCallback (const gchar * url, gboolean success, gpointer user_data)
{
    if (!success) {
        qWarning() << "Fail to launch url:" << url;
    }
}

bool ApplicationUtils::openUrl(const QUrl &url)
{
    if (qgetenv("TELEPHONY_SERVICE_TEST").isEmpty()) {
        url_dispatch_send(url.toString().toUtf8().constData(), urlDispatchCallback, 0);
    }
    return true;
}

#endif
