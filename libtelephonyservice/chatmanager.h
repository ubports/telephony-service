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

    Q_INVOKABLE QString sendMessage(const QString &accountId, const QString &message, const QVariant &attachments = QVariant(), const QVariantMap &properties = QVariantMap());
    Q_INVOKABLE ChatEntry *chatEntryForProperties(const QString &accountId, const QVariantMap &properties, bool create = false);

    QQmlListProperty<ChatEntry> chats();
    static int chatCount(QQmlListProperty<ChatEntry> *p);
    static ChatEntry* chatAt(QQmlListProperty<ChatEntry> *p, int index);

Q_SIGNALS:
    void messageReceived(const QString &sender, const QString &message, const QDateTime &timestamp, const QString &messageId, bool unread);
    void messageSent(const QStringList &recipients, const QString &message);
    void chatsChanged();
    void chatEntryCreated(QString accountId, QStringList participants, ChatEntry *chatEntry);

public Q_SLOTS:
    void onTextChannelAvailable(Tp::TextChannelPtr channel);
    void onChannelInvalidated();
    void onConnectedChanged();
    void onMessageReceived(const Tp::ReceivedMessage &message);
    void onMessageSent(const Tp::Message &sentMessage, const Tp::MessageSendingFlags flags, const QString &message);

    void acknowledgeMessage(const QStringList &recipients, const QString &messageId, const QString &accountId);
    void acknowledgeAllMessages(const QStringList &recipients, const QString &accountId);

private Q_SLOTS:
    void onChannelObserverUnregistered();
    void onTelepathyReady();

protected Q_SLOTS:
    void onAckTimerTriggered();

private:
    explicit ChatManager(QObject *parent = 0);
    ChatEntry *chatEntryForChannel(const Tp::TextChannelPtr &channel);
    QList<ChatEntry*> chatEntries() const;

    mutable QList<ChatEntry*> mChatEntries;
    QMap<QString, QMap<QStringList,QStringList> > mMessagesToAck;
    QList<Tp::TextChannelPtr> mPendingChannels;
    QTimer mMessagesAckTimer;
    bool mReady;
};

#endif // CHATMANAGER_H
