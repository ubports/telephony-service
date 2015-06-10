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

#include "protocolmanager.h"
#include "config.h"
#include <QDir>

ProtocolManager *ProtocolManager::instance()
{
    static ProtocolManager self;
    return &self;
}

Protocols ProtocolManager::protocols() const
{
    return mProtocols;
}

QStringList ProtocolManager::protocolNames() const
{
    QStringList names;
    Q_FOREACH(const Protocol *protocol, mProtocols) {
        names << protocol->name();
    }
    return names;
}

Protocols ProtocolManager::protocolsForFeatures(Protocol::Features features) const
{
    Protocols filtered;
    Q_FOREACH(Protocol *protocol, mProtocols) {
        if (protocol->features() & features) {
            filtered << protocol;
        }
    }
    return filtered;
}

Protocols ProtocolManager::textProtocols() const
{
    return protocolsForFeatures(Protocol::TextChats);
}

Protocols ProtocolManager::voiceProtocols() const
{
    return protocolsForFeatures(Protocol::VoiceCalls);
}

Protocol *ProtocolManager::protocolByName(const QString &protocolName) const
{
    Q_FOREACH(Protocol *protocol, mProtocols) {
        if (protocol->name() == protocolName) {
            return protocol;
        }
    }
    return 0;
}

bool ProtocolManager::isProtocolSupported(const QString &protocolName) const
{
    return protocolByName(protocolName) != 0;
}

QQmlListProperty<Protocol> ProtocolManager::qmlProtocols()
{
    return QQmlListProperty<Protocol>(this, 0, qmlProtocolsCount, qmlProtocolsAt);
}

int ProtocolManager::qmlProtocolsCount(QQmlListProperty<Protocol> *p)
{
    Q_UNUSED(p);
    return ProtocolManager::instance()->protocols().count();
}

Protocol *ProtocolManager::qmlProtocolsAt(QQmlListProperty<Protocol> *p, int index)
{
    Q_UNUSED(p);
    return ProtocolManager::instance()->protocols()[index];
}

QQmlListProperty<Protocol> ProtocolManager::qmlTextProtocols()
{
    return QQmlListProperty<Protocol>(this, 0, qmlTextProtocolsCount, qmlTextProtocolsAt);
}

int ProtocolManager::qmlTextProtocolsCount(QQmlListProperty<Protocol> *p)
{
    Q_UNUSED(p);
    return ProtocolManager::instance()->textProtocols().count();
}

Protocol *ProtocolManager::qmlTextProtocolsAt(QQmlListProperty<Protocol> *p, int index)
{
    Q_UNUSED(p);
    return ProtocolManager::instance()->textProtocols()[index];
}

QQmlListProperty<Protocol> ProtocolManager::qmlVoiceProtocols()
{
    return QQmlListProperty<Protocol>(this, 0, qmlVoiceProtocolsCount, qmlVoiceProtocolsAt);
}

int ProtocolManager::qmlVoiceProtocolsCount(QQmlListProperty<Protocol> *p)
{
    Q_UNUSED(p);
    return ProtocolManager::instance()->voiceProtocols().count();
}

Protocol *ProtocolManager::qmlVoiceProtocolsAt(QQmlListProperty<Protocol> *p, int index)
{
    Q_UNUSED(p);
    return ProtocolManager::instance()->voiceProtocols()[index];
}

void ProtocolManager::loadSupportedProtocols()
{
    QDir dir(protocolsDir());
    Q_FOREACH(QString entry, dir.entryList()) {
        if (!entry.endsWith(".protocol")) {
            continue;
        }
        Protocol *protocol = Protocol::fromFile(dir.absoluteFilePath(entry));
        if (protocol) {
            protocol->setParent(this);
            mProtocols << protocol;
        }
    }

    Q_EMIT protocolsChanged();
}

ProtocolManager::ProtocolManager(QObject *parent) :
    QObject(parent)
{
    loadSupportedProtocols();
}
