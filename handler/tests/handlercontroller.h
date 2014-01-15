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
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 */

#ifndef HANDLERCONTROLLER_H
#define HANDLERCONTROLLER_H

#include <QObject>
#include <QDBusInterface>

class HandlerController : public QObject
{
    Q_OBJECT
public:
    static HandlerController *instance();

public Q_SLOTS:
    void startCall(const QString &number);
    void hangUpCall(const QString &objectPath);
    void setHold(const QString &objectPath, bool hold);
    void setMuted(const QString &objectPath, bool muted);
    void setSpeakerMode(const QString &objectPath, bool enabled);
    void sendDTMF(const QString &objectPath, const QString &key);

    void sendMessage(const QString &number, const QString &message);
    void acknowledgeMessages(const QString &number, const QStringList &messageIds);

private:
    explicit HandlerController(QObject *parent = 0);
    QDBusInterface mHandlerInterface;
};

#endif // HANDLERCONTROLLER_H
