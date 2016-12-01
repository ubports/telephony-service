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
#include "telepathyhelper.h"
#include "config.h"
#include "dbustypes.h"
#include <QDir>
#include <QDBusMetaType>

QDBusArgument &operator<<(QDBusArgument &argument, const ProtocolStruct &protocol)
{
    argument.beginStructure();
    argument << protocol.name << protocol.features << protocol.fallbackProtocol << protocol.backgroundImage << protocol.icon << protocol.serviceName;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ProtocolStruct &protocol)
{
    argument.beginStructure();
    argument >> protocol.name >> protocol.features >> protocol.fallbackProtocol >> protocol.backgroundImage >> protocol.icon >> protocol.serviceName;
    argument.endStructure();
    return argument;
}

/**
 * Protocol Manager acts in two senses. If protocols dir is accessible it takes info from there and exposes it thtrough DBus (handler will be accessible
 * in this case). Otherwise, it queries for protocols through DBus, avoiding accessing disk to get that information. This last is useful in confined
 * environments where info could only be accessible by DBus
 */
ProtocolManager::ProtocolManager(const QString &dir, QObject *parent) :
    QObject(parent), mProtocolsDir(dir)
{
    QDir d(mProtocolsDir);
    // read from disk and emit signal of available protocols in case protocols directory exists (We are servers)
    if (d.exists()) {
        mFileWatcher.addPath(mProtocolsDir);
        connect(&mFileWatcher,
                SIGNAL(directoryChanged(QString)),
                SLOT(loadSupportedProtocols()));
        loadSupportedProtocols();
    } else {
        // register DBus types and query protocols info through DBus in case protocols directory does not exists (We are clients)
        qDBusRegisterMetaType<ProtocolList>();
        qDBusRegisterMetaType<ProtocolStruct>();

        //TODO make DBus call to get the protocols
        QDBusInterface *interface = TelepathyHelper::instance()->handlerInterface();

        if (!interface) {
            return;
        }

        connect(interface,
                SIGNAL(ProtocolsChanged(ProtocolList)),
                SLOT(onProtocolsChanged(ProtocolList)));

        QDBusReply<ProtocolList> reply = interface->call("GetProtocols");
        if (!reply.isValid()) {
            return;
        }

        mProtocols.clear();
        Q_FOREACH (const ProtocolStruct& protocol, reply.value()) {
            mProtocols << new Protocol(protocol);
        }
    }
}

ProtocolManager *ProtocolManager::instance()
{
    static ProtocolManager self(protocolsDir());
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
    // clear previous entries
    Q_FOREACH(Protocol *protocol, mProtocols) {
        protocol->deleteLater();
    }
    mProtocols.clear();

    // and scan the directory
    QDir dir(mProtocolsDir);
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

void ProtocolManager::onProtocolsChanged(const ProtocolList &protocolList)
{
    mProtocols.clear();
    Q_FOREACH (const ProtocolStruct &protocol, protocolList) {
        mProtocols << new Protocol(protocol);
    }

    Q_EMIT protocolsChanged();
}
