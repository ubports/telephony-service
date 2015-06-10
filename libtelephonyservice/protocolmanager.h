/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#ifndef PROTOCOLMANAGER_H
#define PROTOCOLMANAGER_H

#include <QObject>
#include <QQmlListProperty>
#include "protocol.h"

/// @brief Manages the list of supported protocols
class ProtocolManager : public QObject
{
    Q_OBJECT
    /// @brief all supported protocols
    Q_PROPERTY(QQmlListProperty<Protocol> protocols
               READ qmlProtocols
               NOTIFY protocolsChanged)
    /// @brief protocols that support text chats
    Q_PROPERTY(QQmlListProperty<Protocol> textProtocols
               READ qmlTextProtocols
               NOTIFY protocolsChanged)
    /// @brief protocols that support voice calls
    Q_PROPERTY(QQmlListProperty<Protocol> voiceProtocols
               READ qmlVoiceProtocols
               NOTIFY protocolsChanged)
    /// @brief the name of all supported protocols
    Q_PROPERTY(QStringList protocolNames READ protocolNames NOTIFY protocolsChanged)

public:
    static ProtocolManager *instance();

    /// @brief returns all supported protocols
    Protocols protocols() const;

    /// @brief return the name of all supported protocols
    QStringList protocolNames() const;

    /// @brief returns all protocols matching the given flags
    Protocols protocolsForFeatures(Protocol::Features features) const;

    /// @brief convenience function returning all protocols that support text chats
    Protocols textProtocols() const;

    /// @brief convenience function returning all protocols that support voice calls
    Protocols voiceProtocols() const;

    /// @brief returns the protocol information for the given @ref protocolName or 0 if not supported
    Protocol *protocolByName(const QString &protocolName) const;

    /// @brief checks if a given @ref protocolName is supported
    bool isProtocolSupported(const QString &protocolName) const;

    // QML protocols property helpers
    QQmlListProperty<Protocol> qmlProtocols();
    static int qmlProtocolsCount(QQmlListProperty<Protocol> *p);
    static Protocol *qmlProtocolsAt(QQmlListProperty<Protocol> *p, int index);

    // QML textProtocols property helpers
    QQmlListProperty<Protocol> qmlTextProtocols();
    static int qmlTextProtocolsCount(QQmlListProperty<Protocol> *p);
    static Protocol *qmlTextProtocolsAt(QQmlListProperty<Protocol> *p, int index);

    // QML voiceProtocols property helpers
    QQmlListProperty<Protocol> qmlVoiceProtocols();
    static int qmlVoiceProtocolsCount(QQmlListProperty<Protocol> *p);
    static Protocol *qmlVoiceProtocolsAt(QQmlListProperty<Protocol> *p, int index);

Q_SIGNALS:
    void protocolsChanged();

protected Q_SLOTS:
    void loadSupportedProtocols();

private:
    explicit ProtocolManager(QObject *parent = 0);
    Protocols mProtocols;

};

#endif // PROTOCOLMANAGER_H
