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

    QVariantMap getCallProperties(const QString &objectPath);

public Q_SLOTS:
    // call methods
    void startCall(const QString &number, const QString &accountId);
    void hangUpCall(const QString &objectPath);
    void setHold(const QString &objectPath, bool hold);
    void setMuted(const QString &objectPath, bool muted);
    void setSpeakerMode(const QString &objectPath, bool enabled);
    void sendDTMF(const QString &objectPath, const QString &key);

    // conference call methods
    void createConferenceCall(const QStringList &objectPaths);
    void mergeCall(const QString &conferenceObjectPath, const QString &callObjectPath);
    void splitCall(const QString &objectPath);

    // messaging methods
    void sendMessage(const QString &number, const QString &message, const QString &accountId);
    void acknowledgeMessages(const QString &number, const QStringList &messageIds, const QString &accountId);

Q_SIGNALS:
    void callPropertiesChanged(const QString &objectPath, const QVariantMap &properties);

private:
    explicit HandlerController(QObject *parent = 0);
    QDBusInterface mHandlerInterface;
};

#endif // HANDLERCONTROLLER_H
