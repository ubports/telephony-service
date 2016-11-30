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

#include "protocolwatcher.h"
#include "config.h"
#include <QDir>

ProtocolWatcher::ProtocolWatcher(const QString &dir, QObject *parent) :
    QObject(parent), mProtocolsDir(dir)
{
    mFileWatcher.addPath(mProtocolsDir);
    connect(&mFileWatcher,
            SIGNAL(directoryChanged(QString)),
            SLOT(loadSupportedProtocols()));
    loadSupportedProtocols();
}

ProtocolWatcher *ProtocolWatcher::instance()
{
    static ProtocolWatcher self(protocolsDir());
    return &self;
}

ProtocolList ProtocolWatcher::protocols() const
{
    return mProtocols;
}

void ProtocolWatcher::loadSupportedProtocols()
{
    if (mProtocolsDir.isEmpty()) {
        return;
    }

    QDir dir(mProtocolsDir);
    if (!dir.exists()) {
        return;
    }

    // scan the directory searching for protocol files
    Q_FOREACH(QString entry, dir.entryList()) {
        if (!entry.endsWith(".protocol")) {
            continue;
        }
        Protocol *protocol = Protocol::fromFile(dir.absoluteFilePath(entry));
        if (protocol) {
            protocol->setParent(this);
            mProtocols << protocol->dbusType();
        }
    }

    Q_EMIT protocolsChanged(mProtocols);
}
