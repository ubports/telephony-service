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

#ifndef PROTOCOLWATCHER_H
#define PROTOCOLWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QQmlListProperty>
#include "protocol.h"

/// @brief Manages the list of supported protocols
class ProtocolWatcher : public QObject
{
    Q_OBJECT
public:
    static ProtocolWatcher *instance();
    ProtocolList protocols() const;

Q_SIGNALS:
    void protocolsChanged(const ProtocolList &protocols);

private:
    void loadSupportedProtocols();
    explicit ProtocolWatcher(const QString &dir, QObject *parent = 0);

private:
    ProtocolList mProtocols;
    QFileSystemWatcher mFileWatcher;
    QString mProtocolsDir;
};

#endif // PROTOCOLMANAGER_H
