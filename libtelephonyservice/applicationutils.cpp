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
#include <QDebug>
#include <QProcess>
#include <TelepathyQt/Constants>

#ifdef USE_UBUNTU_PLATFORM_API
#include <ubuntu/ui/ubuntu_ui_session_service.h>
#endif

#define ADDRESSBOOK_APP_SERVICE "com.canonical.AddressBookApp"
#define DIALER_APP_SERVICE "com.canonical.DialerApp"
#define MESSAGING_APP_SERVICE "com.canonical.MessagingApp"

ApplicationUtils::ApplicationUtils(QObject *parent) :
    QObject(parent)
{
    // Setup a DBus watcher to check if the telephony-service is running
    mApplicationWatcher.setConnection(QDBusConnection::sessionBus());
    mApplicationWatcher.setWatchMode(QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration);
    mApplicationWatcher.addWatchedService(ADDRESSBOOK_APP_SERVICE);
    mApplicationWatcher.addWatchedService(DIALER_APP_SERVICE);
    mApplicationWatcher.addWatchedService(MESSAGING_APP_SERVICE);

    connect(&mApplicationWatcher,
            SIGNAL(serviceRegistered(const QString&)),
            SLOT(onServiceRegistered(const QString&)));
    connect(&mApplicationWatcher,
            SIGNAL(serviceUnregistered(const QString&)),
            SLOT(onServiceUnregistered(const QString&)));


    mAddressbookAppRunning = checkApplicationRunning(ADDRESSBOOK_APP_SERVICE);
    mDialerAppRunning = checkApplicationRunning(DIALER_APP_SERVICE);
    mMessagingAppRunning = checkApplicationRunning(MESSAGING_APP_SERVICE);
}

ApplicationUtils *ApplicationUtils::instance()
{
    static ApplicationUtils *self = new ApplicationUtils();
    return self;
}

void ApplicationUtils::onServiceRegistered(const QString &serviceName)
{
    if (serviceName == ADDRESSBOOK_APP_SERVICE) {
        mAddressbookAppRunning = true;
        Q_EMIT addressbookAppRunningChanged(true);
    } else if (serviceName == DIALER_APP_SERVICE) {
        mDialerAppRunning = true;
        Q_EMIT dialerAppRunningChanged(true);
    } else if (serviceName == MESSAGING_APP_SERVICE) {
        mMessagingAppRunning = true;
        Q_EMIT messagingAppRunningChanged(true);
    }
}

void ApplicationUtils::onServiceUnregistered(const QString &serviceName)
{
    if (serviceName == ADDRESSBOOK_APP_SERVICE) {
        mAddressbookAppRunning = false;
        Q_EMIT addressbookAppRunningChanged(false);
    } else if (serviceName == DIALER_APP_SERVICE) {
        mDialerAppRunning = false;
        Q_EMIT dialerAppRunningChanged(false);
    } else if (serviceName == MESSAGING_APP_SERVICE) {
        mMessagingAppRunning = false;
        Q_EMIT messagingAppRunningChanged(false);
    }
}

void ApplicationUtils::switchToAddressbookApp(const QString &argument)
{
    qDebug() << "Starting address-book-app...";
#ifdef USE_UBUNTU_PLATFORM_API
    ubuntu_ui_session_trigger_switch_to_well_known_application(ADDRESSBOOK_APP);
#else
    if (!mAddressbookAppRunning) {
        QProcess::startDetached("address-book-app");
    }
#endif

    // block until the app is registered
    while (!mAddressbookAppRunning) {
        QCoreApplication::processEvents();
    }

    if (!argument.isEmpty()) {
        QDBusInterface addressbookApp("com.canonical.AddressBookApp",
                                    "/com/canonical/AddressBookApp",
                                    "com.canonical.AddressBookApp");
        addressbookApp.call("SendAppMessage", argument);
    }

    qDebug() << "... succeeded!";
}

void ApplicationUtils::switchToDialerApp(const QString &argument)
{
    qDebug() << "Starting dialer-app...";
#ifdef USE_UBUNTU_PLATFORM_API
    ubuntu_ui_session_trigger_switch_to_well_known_application(DIALER_APP);
#else
    if (!mDialerAppRunning) {
        QProcess::startDetached("dialer-app");
    }
#endif

    // block until the app is registered
    while (!mDialerAppRunning) {
        QCoreApplication::processEvents();
    }

    if (!argument.isEmpty()) {
        QDBusInterface dialerApp("com.canonical.DialerApp",
                                 "/com/canonical/DialerApp",
                                 "com.canonical.DialerApp");
        dialerApp.call("SendAppMessage", argument);
    }

    qDebug() << "... succeeded!";
}

void ApplicationUtils::switchToMessagingApp(const QString &argument)
{
    qDebug() << "Starting messaging-app...";
#ifdef USE_UBUNTU_PLATFORM_API
    ubuntu_ui_session_trigger_switch_to_well_known_application(MESSAGING_APP);
#else
    if (!mMessagingAppRunning) {
        QProcess::startDetached("messaging-app");
    }
#endif

    // block until the app is registered
    while (!mMessagingAppRunning) {
        QCoreApplication::processEvents();
    }

    if (!argument.isEmpty()) {
        QDBusInterface messagingApp("com.canonical.MessagingApp",
                                    "/com/canonical/MessagingApp",
                                    "com.canonical.MessagingApp");
        messagingApp.call("SendAppMessage", argument);
    }

    qDebug() << "... succeeded!";
}

bool ApplicationUtils::addressbookAppRunning()
{
    return mAddressbookAppRunning;
}

bool ApplicationUtils::dialerAppRunning()
{
    return mDialerAppRunning;
}

bool ApplicationUtils::messagingAppRunning()
{
    return mMessagingAppRunning;
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
