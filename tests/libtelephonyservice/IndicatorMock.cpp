/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include <QObject>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDebug>

#define INDICATOR_DBUS_SERVICE_NAME "com.canonical.TelephonyServiceIndicator"
#define INDICATOR_DBUS_OBJ_PATH "/com/canonical/TelephonyServiceIndicator"

class IndicatorMock : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", INDICATOR_DBUS_SERVICE_NAME)

public:
    Q_SCRIPTABLE void ClearNotifications();
    Q_SCRIPTABLE void ClearCallNotification(const QString &targetId, const QString &accountId);

Q_SIGNALS:
    Q_SCRIPTABLE void ClearNotificationsRequested();
    Q_SCRIPTABLE void ClearCallNotificationRequested(const QString &targetId, const QString &accountId);
};

void IndicatorMock::ClearNotifications()
{
    Q_EMIT ClearNotificationsRequested();
}

void IndicatorMock::ClearCallNotification(const QString &targetId, const QString &accountId)
{
    Q_EMIT ClearCallNotificationRequested(targetId, accountId);
}

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    QDBusConnection connection = QDBusConnection::sessionBus();

    IndicatorMock toneGen;
    connection.registerObject(INDICATOR_DBUS_OBJ_PATH, &toneGen, QDBusConnection::ExportScriptableContents);
    connection.registerService(INDICATOR_DBUS_SERVICE_NAME);

    return a.exec();
}

#include "IndicatorMock.moc"
