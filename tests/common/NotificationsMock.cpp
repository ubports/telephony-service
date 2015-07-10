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

#define NOTIFICATIONS_DBUS_SERVICE_NAME "org.freedesktop.Notifications"
#define NOTIFICATIONS_DBUS_OBJ_PATH "/org/freedesktop/Notifications"

class NotificationsMock : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", NOTIFICATIONS_DBUS_SERVICE_NAME)

public:
    Q_SCRIPTABLE uint Notify(QString app_name, uint replaces_id, QString app_icon, QString summary, QString body, QStringList actions, QVariantMap hints, int expire_timeout);
    Q_SCRIPTABLE void CloseNotification(uint id);
    Q_SCRIPTABLE QString GetServerInformation(QString& vendor, QString& version, QString& spec_version);

    // Mock specific method
    Q_SCRIPTABLE void MockInvokeAction(uint id, QString action_key);

Q_SIGNALS:
    Q_SCRIPTABLE void NotificationClosed(uint id, uint reason);
    Q_SCRIPTABLE void ActionInvoked(uint id, QString action_key);

    // Mock specific signal
    Q_SCRIPTABLE void MockNotificationReceived(QString app_name, uint replaces_id, QString app_icon, QString summary, QString body, QStringList actions, QVariantMap hints, int expire_timeout);
};

uint NotificationsMock::Notify(QString app_name, uint replaces_id, QString app_icon, QString summary, QString body, QStringList actions, QVariantMap hints, int expire_timeout)
{
    Q_EMIT MockNotificationReceived(app_name, replaces_id, app_icon, summary, body, actions, hints, expire_timeout);
    static uint id = 1;
    return (replaces_id != 0 ? replaces_id : id++);
}

void NotificationsMock::MockInvokeAction(uint id, QString action_key)
{
    Q_EMIT ActionInvoked(id, action_key);
    Q_EMIT NotificationClosed(id, 2); // 2 is dismissed by user
}

void NotificationsMock::CloseNotification(uint id)
{
    Q_EMIT NotificationClosed(id, 3); // 3 is closed by a CloseNotification() call
}

QString NotificationsMock::GetServerInformation(QString &vendor, QString &version, QString &spec_version)
{
    return QString();
}

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    QDBusConnection connection = QDBusConnection::sessionBus();

    NotificationsMock notifications;
    connection.registerObject(NOTIFICATIONS_DBUS_OBJ_PATH, &notifications, QDBusConnection::ExportScriptableContents);
    connection.registerService(NOTIFICATIONS_DBUS_SERVICE_NAME);

    return a.exec();
}

#include "NotificationsMock.moc"
