/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * Authors:
 *  Roberto Mier Escandon <roberto.escandon@canonical.com>
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * history-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libtelephonyservice/rolesinterface.h>

ChannelInterfaceRolesInterface::ChannelInterfaceRolesInterface(const QString& busName, const QString& objectPath, QObject *parent)
    : Tp::AbstractInterface(busName, objectPath, staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
}

ChannelInterfaceRolesInterface::ChannelInterfaceRolesInterface(const QDBusConnection& connection, const QString& busName, const QString& objectPath, QObject *parent)
    : Tp::AbstractInterface(busName, objectPath, staticInterfaceName(), connection, parent)
{
}

ChannelInterfaceRolesInterface::ChannelInterfaceRolesInterface(Tp::DBusProxy *proxy)
    : Tp::AbstractInterface(proxy, staticInterfaceName())
{
}

ChannelInterfaceRolesInterface::ChannelInterfaceRolesInterface(const Tp::Client::ChannelInterface& mainInterface)
    : Tp::AbstractInterface(mainInterface.service(), mainInterface.path(), staticInterfaceName(), mainInterface.connection(), mainInterface.parent())
{
}

ChannelInterfaceRolesInterface::ChannelInterfaceRolesInterface(const Tp::Client::ChannelInterface& mainInterface, QObject *parent)
    : Tp::AbstractInterface(mainInterface.service(), mainInterface.path(), staticInterfaceName(), mainInterface.connection(), parent)
{
}

void ChannelInterfaceRolesInterface::invalidate(Tp::DBusProxy *proxy,
        const QString &error, const QString &message)
{
    Tp::AbstractInterface::invalidate(proxy, error, message);
}

HandleRolesMap ChannelInterfaceRolesInterface::getRoles() const
{
    QDBusMessage msg = QDBusMessage::createMethodCall(service(), path(),
            TP_QT_IFACE_PROPERTIES, QLatin1String("Get"));
    msg << interface() << QLatin1String("Roles");
    QDBusMessage result = connection().call(msg);
    return qdbus_cast<HandleRolesMap>(result.arguments().at(0).value<QDBusVariant>().variant());
}

bool ChannelInterfaceRolesInterface::getCanUpdateRoles() const
{
    QDBusMessage msg = QDBusMessage::createMethodCall(service(), path(),
            TP_QT_IFACE_PROPERTIES, QLatin1String("Get"));
    msg << interface() << QLatin1String("CanUpdateRoles");
    QDBusMessage result = connection().call(msg);
    return qdbus_cast<bool>(result.arguments().at(0).value<QDBusVariant>().variant());
}
