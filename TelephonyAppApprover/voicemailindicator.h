/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VOICEMAILINDICATOR_H
#define VOICEMAILINDICATOR_H

#include <QDBusInterface>
#include <QVariantMap>
#include <QStringList>

namespace QIndicate {
    class Indicator;
    class Server;
}

class VoiceMailIndicator : public QObject
{
    Q_OBJECT
public:
    explicit VoiceMailIndicator(QObject *parent = 0);

protected:
    void updateCounter();

public Q_SLOTS:
    void onPropertiesChanged(const QString &interfaceName,
                             const QVariantMap &changedProperties,
                             const QStringList &invalidatedProperties);
    void onIndicatorDisplay(QIndicate::Indicator *indicator);

private:
    QDBusConnection mConnection;
    QIndicate::Server *mIndicateServer;
    QIndicate::Indicator *mIndicator;
    QDBusInterface mInterface;
};

#endif // VOICEMAILINDICATOR_H
