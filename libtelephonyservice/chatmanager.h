/*
 * Copyright (C) 2012 Canonical, Ltd.
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
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>
#include "dbustypes.h"

class ChatManager : public QObject
{
    Q_OBJECT
public:
    static ChatManager *instance();

    Q_INVOKABLE void sendMessage(const QStringList &phoneNumbers, const QString &message, const QString &accountId = QString::null);
    Q_INVOKABLE void sendMMS(const QStringList &phoneNumbers, const QString &message, const QVariant &attachments, const QString &accountId = QString:: null);

Q_SIGNALS:
    void messageReceived(const QString &phoneNumber, const QString &message, const QDateTime &timestamp, const QString &messageId, bool unread);
    void messageSent(const QString &phoneNumber, const QString &message);
    void unreadMessagesChanged(const QString &phoneNumber);

public Q_SLOTS:
    void onTextChannelAvailable(Tp::TextChannelPtr channel);
    void onConnectedChanged();
    void onMessageReceived(const Tp::ReceivedMessage &message);
    void onPendingMessageRemoved(const Tp::ReceivedMessage &message);
    void onMessageSent(const Tp::Message &sentMessage, const Tp::MessageSendingFlags flags, const QString &message);

    void acknowledgeMessage(const QStringList &recipients, const QString &messageId, const QString &accountId = QString::null);

protected:
    Tp::TextChannelPtr existingChat(const QStringList &phoneNumbers, const QString &accountId);

protected Q_SLOTS:
    void onAckTimerTriggered();

private:
    explicit ChatManager(QObject *parent = 0);

    QList<Tp::TextChannelPtr> mChannels;
    QMap<QString, QMap<QStringList,QStringList> > mMessagesToAck;
    QTimer mMessagesAckTimer;
};

#endif // CHATMANAGER_H
