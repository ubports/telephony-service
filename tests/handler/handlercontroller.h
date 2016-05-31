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
#include <QDBusArgument>
#include "dbustypes.h"

class HandlerController : public QObject
{
    Q_OBJECT
public:
    static HandlerController *instance();

    QVariantMap getCallProperties(const QString &objectPath);
    bool callIndicatorVisible();

public Q_SLOTS:
    // call methods
    void startChat(const QString &accountId, const QStringList &recipients);
    void startCall(const QString &number, const QString &accountId);
    void hangUpCall(const QString &objectPath);
    void setHold(const QString &objectPath, bool hold);
    void setMuted(const QString &objectPath, bool muted);
    void setSpeakerMode(const QString &objectPath, bool enabled);
    void sendDTMF(const QString &objectPath, const QString &key);
    bool hasCalls();

    // conference call methods
    void createConferenceCall(const QStringList &objectPaths);
    void mergeCall(const QString &conferenceObjectPath, const QString &callObjectPath);
    void splitCall(const QString &objectPath);

    // messaging methods
    QString sendMessage(const QString &accountId, const QStringList &recipients, const QString &message, const AttachmentList &attachments = AttachmentList(), const QVariantMap &properties = QVariantMap());
    void acknowledgeMessages(const QVariantMap &message);

    // active call indicator
    void setCallIndicatorVisible(bool visible);

Q_SIGNALS:
    void callPropertiesChanged(const QString &objectPath, const QVariantMap &properties);
    void callIndicatorVisibleChanged(bool visible);

private:
    explicit HandlerController(QObject *parent = 0);
    QDBusInterface mHandlerInterface;
};

#endif // HANDLERCONTROLLER_H
