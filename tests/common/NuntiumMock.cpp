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

#define NUNTIUM_DBUS_SERVICE_NAME "org.ofono.mms"
#define NUNTIUM_DBUS_OBJ_PATH "/org/ofono/mms"

class NuntiumMock : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", NUNTIUM_DBUS_SERVICE_NAME)

public:
		NuntiumMock(QDBusConnection& conn);
    Q_SCRIPTABLE QString GetServerInformation(QString& vendor, QString& version, QString& spec_version);

Q_SIGNALS:
private:
		QDBusConnection& connection;
};

NuntiumMock::NuntiumMock(QDBusConnection &conn):
	connection(conn) {
}


QString NuntiumMock::GetServerInformation(QString &vendor, QString &version, QString &spec_version)
{
    return QString();
}

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    QDBusConnection connection = QDBusConnection::sessionBus();

    NuntiumMock nuntium(connection);
    connection.registerObject(NUNTIUM_DBUS_OBJ_PATH, &nuntium, QDBusConnection::ExportScriptableContents);
    connection.registerService(NUNTIUM_DBUS_SERVICE_NAME);

    return a.exec();
}

#include "NuntiumMock.moc"
