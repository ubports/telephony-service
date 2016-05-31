/*
 * Copyright (C) 2012-2016 Canonical, Ltd.
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

#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QQmlListProperty>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>
#include "dbustypes.h"

class ChatManager : public QObject
{
    Q_OBJECT
public:
    static ChatManager *instance();

    QString sendMessage(const QString &accountId, const QString &message, const QVariant &attachments = QVariant(), const QVariantMap &properties = QVariantMap());
    QList<Tp::TextChannelPtr> channelForProperties(const QVariantMap &properties);
    Tp::TextChannelPtr channelForObjectPath(const QString &objectPath);

Q_SIGNALS:
    void textChannelAvailable(Tp::TextChannelPtr);
    void textChannelInvalidated(Tp::TextChannelPtr);

public Q_SLOTS:
    void onTextChannelAvailable(Tp::TextChannelPtr channel);
    void onChannelInvalidated();
    void onConnectedChanged();

    void acknowledgeMessage(const QStringList &recipients, const QString &messageId, const QString &accountId);
    void acknowledgeAllMessages(const QStringList &recipients, const QString &accountId);

private Q_SLOTS:
    void onChannelObserverUnregistered();

protected Q_SLOTS:
    void onAckTimerTriggered();

private:
    explicit ChatManager(QObject *parent = 0);

    QMap<QString, QMap<QStringList,QStringList> > mMessagesToAck;
    QList<Tp::TextChannelPtr> mTextChannels;
    QTimer mMessagesAckTimer;
};

#endif // CHATMANAGER_H
