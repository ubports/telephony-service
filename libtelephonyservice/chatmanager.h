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
#include <QQmlListProperty>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>
#include "dbustypes.h"
#include "chatentry.h"

class ChatManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<ChatEntry> chats
                   READ chats
                   NOTIFY chatsChanged)
public:
    static ChatManager *instance();

    Q_INVOKABLE void sendMessage(const QStringList &recipients, const QString &message, const QString &accountId = QString::null);
    Q_INVOKABLE void sendMMS(const QStringList &recipients, const QString &message, const QVariant &attachments, const QString &accountId = QString:: null);
    Q_INVOKABLE ChatEntry *chatEntryForParticipants(const QString &accountId, const QStringList &participants, bool create);
    Q_INVOKABLE ChatEntry *chatEntryForChatId(const QString &accountId, const QString &chatId, bool create);

    QQmlListProperty<ChatEntry> chats();
    static int chatsCount(QQmlListProperty<ChatEntry> *p);
    static ChatEntry* chatAt(QQmlListProperty<ChatEntry> *p, int index);

Q_SIGNALS:
    void messageReceived(const QString &sender, const QString &message, const QDateTime &timestamp, const QString &messageId, bool unread);
    void messageSent(const QStringList &recipients, const QString &message);
    void chatsChanged();
    void chatEntryCreated(const QString &accountId, const QStringList &participants, const ChatEntry *chatEntry);

public Q_SLOTS:
    void onTextChannelAvailable(Tp::TextChannelPtr channel);
    void onChannelInvalidated();
    void onConnectedChanged();
    void onMessageReceived(const Tp::ReceivedMessage &message);
    void onMessageSent(const Tp::Message &sentMessage, const Tp::MessageSendingFlags flags, const QString &message);

    void acknowledgeMessage(const QStringList &recipients, const QString &messageId, const QString &accountId = QString::null);

protected Q_SLOTS:
    void onAckTimerTriggered();

private:
    explicit ChatManager(QObject *parent = 0);
    ChatEntry *chatEntryForChannel(const Tp::TextChannelPtr &channel);
    QList<ChatEntry*> chatEntries() const;

    mutable QList<ChatEntry*> mChatEntries;
    QMap<QString, QMap<QStringList,QStringList> > mMessagesToAck;
    QTimer mMessagesAckTimer;
};

#endif // CHATMANAGER_H
