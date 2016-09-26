/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * Authors:
 *  Roberto Mier Escandon <roberto.escandon@canonical.com>
 *
 * This file is part of history-service.
 *
 * history-service is free software; you can redistribute it and/or modify
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

#ifndef CHANNELINTERFACEROLESINTERFACE_H
#define CHANNELINTERFACEROLESINTERFACE_H

#include <QMap>

#include <TelepathyQt/AbstractInterface>
#include <TelepathyQt/ChannelInterface>

/**
 * \struct HandleRolesMap
 * \ingroup mapping
 * \headerfile TelepathyQt/types.h <TelepathyQt/Types>
 *
 * Convertible with
 * QMap<uint, uint>, but needed to have a discrete type in the Qt type system.
 *
 * A map from channel-specific handles to their owners.
 */
struct HandleRolesMap : public QMap<uint, uint>
{
    inline HandleRolesMap() : QMap<uint, uint>() {}
    inline HandleRolesMap(const QMap<uint, uint>& a) : QMap<uint, uint>(a) {}

    inline HandleRolesMap& operator=(const QMap<uint, uint>& a)
    {
        *(static_cast<QMap<uint, uint>*>(this)) = a;
        return *this;
    }
};

Q_DECLARE_METATYPE(HandleRolesMap)

class ChannelInterfaceRolesInterface : public Tp::AbstractInterface
{
    Q_OBJECT
public:

    /**
     * Returns the name of the interface "org.freedesktop.Telepathy.Channel.Interface.Roles", which this class
     * represents.
     *
     * \return The D-Bus interface name.
     */
    static inline QLatin1String staticInterfaceName()
    {
        return QLatin1String("org.freedesktop.Telepathy.Channel.Interface.Roles");
    }

    /**
     * Creates a ChannelInterfaceRolesInterface associated with the given object on the session bus.
     *
     * \param busName Name of the service the object is on.
     * \param objectPath Path to the object on the service.
     * \param parent Passed to the parent class constructor.
     */
    ChannelInterfaceRolesInterface(
        const QString& busName,
        const QString& objectPath,
        QObject* parent = 0
    );

    /**
     * Creates a ChannelInterfaceRolesInterface associated with the given object on the given bus.
     *
     * \param connection The bus via which the object can be reached.
     * \param busName Name of the service the object is on.
     * \param objectPath Path to the object on the service.
     * \param parent Passed to the parent class constructor.
     */
    ChannelInterfaceRolesInterface(
        const QDBusConnection& connection,
        const QString& busName,
        const QString& objectPath,
        QObject* parent = 0
    );

    /**
     * Creates a ChannelInterfaceRolesInterface associated with the same object as the given proxy.
     *
     * \param proxy The proxy to use. It will also be the QObject::parent()
     *               for this object.
     */
    ChannelInterfaceRolesInterface(Tp::DBusProxy *proxy);

    /**
     * Creates a ChannelInterfaceRolesInterface associated with the same object as the given proxy.
     * Additionally, the created proxy will have the same parent as the given
     * proxy.
     *
     * \param mainInterface The proxy to use.
     */
    explicit ChannelInterfaceRolesInterface(const Tp::Client::ChannelInterface& mainInterface);

    /**
     * Creates a ChannelInterfaceRolesInterface associated with the same object as the given proxy.
     * However, a different parent object can be specified.
     *
     * \param mainInterface The proxy to use.
     * \param parent Passed to the parent class constructor.
     */
    ChannelInterfaceRolesInterface(const Tp::Client::ChannelInterface& mainInterface, QObject* parent);

    /**
     * Asynchronous getter for the remote object property \c Roles of type \c HandleRolesMap.
     *
     * \return A pending variant which will emit finished when the property has been
     *          retrieved.
     */
    inline Tp::PendingVariant *requestPropertyRoles() const
    {
        return internalRequestProperty(QLatin1String("Roles"));
    }

    /**
     * Asynchronous getter for the remote object property \c CanUpdateRoles of type \c bool.
     *
     * \return A pending variant which will emit finished when the property has been
     *          retrieved.
     */
    inline Tp::PendingVariant *requestPropertyCanUpdateRoles() const
    {
        return internalRequestProperty(QLatin1String("CanUpdateRoles"));
    }

    /**
     * Request all of the DBus properties on the interface.
     *
     * \return A pending variant map which will emit finished when the properties have
     *          been retrieved.
     */
    Tp::PendingVariantMap *requestAllProperties() const
    {
        return internalRequestAllProperties();
    }

    /**
     * Synchronous version to get Roles property
     */
    HandleRolesMap getRoles() const;

    /**
     * Synchronous version to get CanUpdateRoles property
     */
    bool getCanUpdateRoles() const;

public Q_SLOTS:
    /**
     * Begins a call to the D-Bus method \c UpdateRoles on the remote object.
     *
     * Update the roles in the server
     *
     */
    inline QDBusPendingReply<> UpdateRoles(const HandleRolesMap &contactRoles, int timeout = -1)
    {
        if (!invalidationReason().isEmpty()) {
            return QDBusPendingReply<>(QDBusMessage::createError(
                invalidationReason(),
                invalidationMessage()
            ));
        }

        QDBusMessage callMessage = QDBusMessage::createMethodCall(this->service(), this->path(),
                this->staticInterfaceName(), QLatin1String("UpdateRoles"));
        callMessage << QVariant::fromValue(contactRoles);
        return this->connection().asyncCall(callMessage, timeout);
    }

Q_SIGNALS:
    /**
     * Represents the signal \c RolesChanged on the remote object.
     *
     * Emitted when the state the roles of the channel has changed.
     *
     * \param added
     *
     *     map of handles and related roles added
     *
     * \param removed
     *
     *     map of handles and related roles removed
     */
     void RolesChanged(const HandleRolesMap &added, const HandleRolesMap& removed);

protected:
    virtual void invalidate(Tp::DBusProxy *, const QString &, const QString &);

};

#endif // CHANNELINTERFACEROLESINTERFACE_H
