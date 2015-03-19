/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#ifndef CHATENTRY_H
#define CHATENTRY_H

#include <QObject>
#include <TelepathyQt/TextChannel>

class AccountEntry;

class ContactChatState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString contactId READ contactId CONSTANT)
    Q_PROPERTY(int state READ state WRITE setState NOTIFY stateChanged)
public:
    ContactChatState(const QString &contactId, int state) : mContactId(contactId), mState(state) {}
    QString contactId() { return mContactId; }
    int state() { return mState; }
    void setState(int state) {
        mState = state;
        Q_EMIT stateChanged();
    }
Q_SIGNALS:
    void stateChanged();
private:
    QString mContactId;
    int mState;
};

typedef QList<ContactChatState* > ContactChatStates;

class ChatEntry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(AccountEntry* account READ account CONSTANT)
    Q_PROPERTY(ChatType chatType READ chatType CONSTANT)
    Q_PROPERTY(QStringList participants READ participants NOTIFY participantsChanged)
    Q_PROPERTY(QQmlListProperty<ContactChatState> chatStates
               READ chatStates
               NOTIFY chatStatesChanged)

    Q_ENUMS(ChatType)
public:
    enum ChatType {
        ChatTypeNone    = Tp::HandleTypeNone,
        ChatTypeContact = Tp::HandleTypeContact,
        ChatTypeRoom    = Tp::HandleTypeRoom
    };

    explicit ChatEntry(const Tp::TextChannelPtr &channel, QObject *parent = 0);
    ~ChatEntry();
    Tp::TextChannelPtr channel();
    AccountEntry *account();
    QQmlListProperty<ContactChatState> chatStates();
    QStringList participants();
    ChatType chatType();
    static int chatStatesCount(QQmlListProperty<ContactChatState> *p);
    static ContactChatState *chatStatesAt(QQmlListProperty<ContactChatState> *p, int index);

private Q_SLOTS:
    void onChatStateChanged(const Tp::ContactPtr &contact, Tp::ChannelChatState state);

Q_SIGNALS:
    void chatStatesChanged();
    void participantsChanged();

private:
    AccountEntry *mAccount;
    Tp::TextChannelPtr mChannel;
    QMap<QString, ContactChatState*> mChatStates;
};

#endif // CHATENTRY_H
