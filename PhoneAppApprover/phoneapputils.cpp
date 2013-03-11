/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
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

#include "phoneapputils.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDebug>
#include <TelepathyQt/Constants>

#ifdef USE_UBUNTU_PLATFORM_API
#include <ubuntu/ui/ubuntu_ui_session_service.h>
#endif

#define PHONE_APP_CLIENT TP_QT_IFACE_CLIENT + ".PhoneApp"

PhoneAppUtils::PhoneAppUtils(QObject *parent) :
    QObject(parent)
{
    // Setup a DBus watcher to check if the phone-app is running
    mPhoneAppWatcher.setConnection(QDBusConnection::sessionBus());
    mPhoneAppWatcher.setWatchMode(QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration);
    mPhoneAppWatcher.addWatchedService(PHONE_APP_CLIENT);

    connect(&mPhoneAppWatcher,
            SIGNAL(serviceRegistered(const QString&)),
            SLOT(onServiceRegistered(const QString&)));
    connect(&mPhoneAppWatcher,
            SIGNAL(serviceUnregistered(const QString&)),
            SLOT(onServiceUnregistered(const QString&)));

    QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered(PHONE_APP_CLIENT);
    if (reply.isValid()) {
        mPhoneAppRunning = reply.value();
    } else {
        mPhoneAppRunning = false;
    }
}


PhoneAppUtils *PhoneAppUtils::instance()
{
    static PhoneAppUtils *self = new PhoneAppUtils();
    return self;
}

void PhoneAppUtils::onServiceRegistered(const QString &serviceName)
{
    // for now we are only watching the phone-app service, so no need to use/compare the
    // service name
    Q_UNUSED(serviceName)

    mPhoneAppRunning = true;
    Q_EMIT applicationRunningChanged(mPhoneAppRunning);
}

void PhoneAppUtils::onServiceUnregistered(const QString &serviceName)
{
    // for now we are only watching the phone-app service, so no need to use/compare the
    // service name
    Q_UNUSED(serviceName)

    mPhoneAppRunning = false;
    Q_EMIT applicationRunningChanged(mPhoneAppRunning);
}

void PhoneAppUtils::startPhoneApp()
{
#ifdef USE_UBUNTU_PLATFORM_API
    qDebug() << "Starting phone-app...";
    ubuntu_ui_session_trigger_switch_to_well_known_application(PHONE_APP);

    // block until the app is registered
    while (!mPhoneAppRunning) {
        QCoreApplication::processEvents();
    }

    qDebug() << "... succeeded!";
#endif
}

bool PhoneAppUtils::isApplicationRunning()
{
    return mPhoneAppRunning;
}
