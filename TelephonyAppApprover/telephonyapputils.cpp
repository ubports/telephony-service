/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "telephonyapputils.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDebug>
#include <TelepathyQt/Constants>

#ifdef USE_HYBRIS
#include <hybris/ubuntu/ui/ubuntu_ui_session_service.h>
#endif

#define TELEPHONY_APP_CLIENT TP_QT_IFACE_CLIENT + ".TelephonyApp"

TelephonyAppUtils::TelephonyAppUtils(QObject *parent) :
    QObject(parent)
{
    // Setup a DBus watcher to check if the telephony-app is running
    mTelephonyAppWatcher.setConnection(QDBusConnection::sessionBus());
    mTelephonyAppWatcher.setWatchMode(QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration);
    mTelephonyAppWatcher.addWatchedService(TELEPHONY_APP_CLIENT);

    connect(&mTelephonyAppWatcher,
            SIGNAL(serviceRegistered(const QString&)),
            SLOT(onServiceRegistered(const QString&)));
    connect(&mTelephonyAppWatcher,
            SIGNAL(serviceUnregistered(const QString&)),
            SLOT(onServiceUnregistered(const QString&)));

    QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered(TELEPHONY_APP_CLIENT);
    if (reply.isValid()) {
        mTelephonyAppRunning = reply.value();
    } else {
        mTelephonyAppRunning = false;
    }
}


TelephonyAppUtils *TelephonyAppUtils::instance()
{
    static TelephonyAppUtils *self = new TelephonyAppUtils();
    return self;
}

void TelephonyAppUtils::onServiceRegistered(const QString &serviceName)
{
    // for now we are only watching the telephony-app service, so no need to use/compare the
    // service name
    Q_UNUSED(serviceName)

    mTelephonyAppRunning = true;
    Q_EMIT applicationRunningChanged(mTelephonyAppRunning);
}

void TelephonyAppUtils::onServiceUnregistered(const QString &serviceName)
{
    // for now we are only watching the telephony-app service, so no need to use/compare the
    // service name
    Q_UNUSED(serviceName)

    mTelephonyAppRunning = false;
    Q_EMIT applicationRunningChanged(mTelephonyAppRunning);
}

void TelephonyAppUtils::startTelephonyApp()
{
#ifdef USE_HYBRIS
    qDebug() << "Starting telephony-app...";
    ubuntu_ui_session_trigger_switch_to_well_known_application(TELEPHONY_APP);

    // block until the app is registered
    while (!mTelephonyAppRunning) {
        QCoreApplication::processEvents();
    }

    qDebug() << "... succeeded!";
#endif
}

bool TelephonyAppUtils::isApplicationRunning()
{
    return mTelephonyAppRunning;
}
