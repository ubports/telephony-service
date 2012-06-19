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

#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>

class ChatManager : public QObject
{
    Q_OBJECT
public:
    explicit ChatManager(QObject *parent = 0);
    
    Q_INVOKABLE bool isChattingToContact(const QString &contactId);
    Q_INVOKABLE void startChat(const QString &contactId);
    Q_INVOKABLE void endChat(const QString &contactId);

    Q_INVOKABLE void sendMessage(const QString &contactId, const QString &message);

signals:
    void chatReady(const QString &contactId);
    void messageReceived(const QString &contactId, const QString &message);
    void messageSent(const QString &contactId, const QString &message);

public Q_SLOTS:
    void onTextChannelAvailable(Tp::TextChannelPtr channel);
    void onContactsAvailable(Tp::PendingOperation *op);
    void onMessageReceived(const Tp::ReceivedMessage &message);


private:
    QMap<QString, Tp::TextChannelPtr> mChannels;
    QMap<QString, Tp::ContactPtr> mContacts;
};

#endif // CHATMANAGER_H
