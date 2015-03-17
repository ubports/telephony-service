/**
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 */

#include <QCoreApplication>

#include <TelepathyQt/BaseConnectionManager>
#include <TelepathyQt/Debug>

#include "protocol.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    Tp::registerTypes();
    Tp::enableDebug(true);
    Tp::enableWarnings(true);

    // create a standard protocol
    Tp::BaseProtocolPtr proto = Tp::BaseProtocol::create<Protocol>(
            QDBusConnection::sessionBus(), QLatin1String("mock"));

    // create a phone protocol
    Tp::BaseProtocolPtr phoneProto = Tp::BaseProtocol::create<Protocol>(
            QDBusConnection::sessionBus(), QLatin1String("ofono"));

    Tp::BaseConnectionManagerPtr cm = Tp::BaseConnectionManager::create(
            QDBusConnection::sessionBus(), QLatin1String("mock"));
    cm->addProtocol(phoneProto);
    cm->addProtocol(proto);
    cm->registerObject();

    return a.exec();
}
