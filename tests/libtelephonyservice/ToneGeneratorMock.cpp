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

#define TONEGEN_DBUS_SERVICE_NAME "com.Nokia.Telephony.Tones"
#define TONEGEN_DBUS_OBJ_PATH "/com/Nokia/Telephony/Tones"

class ToneGeneratorMock : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", TONEGEN_DBUS_SERVICE_NAME)

public:
    Q_SCRIPTABLE void StartEventTone(uint key, int volume, uint duration);
    Q_SCRIPTABLE void StopTone();

Q_SIGNALS:
    Q_SCRIPTABLE void StartEventToneRequested(uint key, int volume, uint duration);
    Q_SCRIPTABLE void StopToneRequested();
};

void ToneGeneratorMock::StartEventTone(uint key, int volume, uint duration)
{
    Q_EMIT StartEventToneRequested(key, volume, duration);
}

void ToneGeneratorMock::StopTone()
{
    Q_EMIT StopToneRequested();
}

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    QDBusConnection connection = QDBusConnection::sessionBus();

    ToneGeneratorMock toneGen;
    connection.registerObject(TONEGEN_DBUS_OBJ_PATH, &toneGen, QDBusConnection::ExportScriptableContents);
    connection.registerService(TONEGEN_DBUS_SERVICE_NAME);

    return a.exec();
}

#include "ToneGeneratorMock.moc"
