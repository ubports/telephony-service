/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#ifndef CHATENTRY_H
#define CHATENTRY_H

#include <QObject>
#include <QQmlParserStatus>
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

class ChatEntry : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(ChatType chatType READ chatType WRITE setChatType NOTIFY chatTypeChanged)
    Q_PROPERTY(QStringList participants READ participants WRITE setParticipants NOTIFY participantsChanged)
    Q_PROPERTY(QString roomName READ roomName WRITE setRoomName NOTIFY roomNameChanged)
    Q_PROPERTY(QString chatId READ chatId WRITE setChatId NOTIFY chatIdChanged)
    Q_PROPERTY(QString accountId READ accountId WRITE setAccountId NOTIFY accountIdChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QQmlListProperty<ContactChatState> chatStates
               READ chatStates
               NOTIFY chatStatesChanged)

    Q_ENUMS(ChatType)
    Q_ENUMS(ChatState)
public:
    enum ChatType {
        ChatTypeNone    = Tp::HandleTypeNone,
        ChatTypeContact = Tp::HandleTypeContact,
        ChatTypeRoom    = Tp::HandleTypeRoom
    };

    enum ChatState {
        ChannelChatStateGone      = Tp::ChannelChatStateGone,
        ChannelChatStateInactive  = Tp::ChannelChatStateInactive,
        ChannelChatStateActive    = Tp::ChannelChatStateActive,
        ChannelChatStatePaused    = Tp::ChannelChatStatePaused,
        ChannelChatStateComposing = Tp::ChannelChatStateComposing
    };

    explicit ChatEntry(QObject *parent = 0);
    ~ChatEntry();
    QQmlListProperty<ContactChatState> chatStates();
    QStringList participants() const;
    void setParticipants(const QStringList &participants);
    ChatType chatType() const;
    void setChatType(ChatType type);
    QString chatId() const;
    void setChatId(const QString &id);
    QString accountId() const;
    void setAccountId(const QString &id);
    QString roomName() const;
    void setRoomName(const QString &name);
    QString title() const;
    void setTitle(const QString & title);
    static int chatStatesCount(QQmlListProperty<ContactChatState> *p);
    static ContactChatState *chatStatesAt(QQmlListProperty<ContactChatState> *p, int index);

    // QML parser status
    void classBegin();
    void componentComplete();

public Q_SLOTS:
    // FIXME: void or return something?
    void sendMessage(const QString &accountId, const QString &message, const QVariant &attachments = QVariant(), const QVariantMap &properties = QVariantMap());
    void setChatState(ChatState state);

    bool destroyRoom();

protected:
    void setChannels(const QList<Tp::TextChannelPtr> &channels);
    void addChannel(const Tp::TextChannelPtr &channel);

    QVariantMap generateProperties() const;

private Q_SLOTS:
    void onTextChannelAvailable(const Tp::TextChannelPtr &channel);
    void onChannelInvalidated();
    void onChatStateChanged(const Tp::ContactPtr &contact, Tp::ChannelChatState state);
    void onRoomPropertiesChanged(const QVariantMap &changed,const QStringList &invalidated);
    void onSendingMessageFinished();

Q_SIGNALS:
    void chatTypeChanged();
    void chatIdChanged();
    void accountIdChanged();
    void chatStatesChanged();
    void participantsChanged();
    void roomNameChanged();
    void titleChanged();

    void messageSent(const QString &accountId, const QString &messageId, const QVariantMap &properties);
    void messageSendingFailed(const QString &accountId, const QString &messageId, const QVariantMap &properties);

private:
    QList<Tp::TextChannelPtr> mChannels;
    QStringList mParticipants;
    QMap<QString, ContactChatState*> mChatStates;
    QString mRoomName;
    QString mTitle;
    QString mChatId;
    QString mAccountId;
    ChatType mChatType;
    Tp::Client::ChannelInterfaceRoomInterface *roomInterface;
    Tp::Client::ChannelInterfaceRoomConfigInterface *roomConfigInterface;
    Tp::Client::ChannelInterfaceSubjectInterface *subjectInterface;
};

#endif // CHATENTRY_H
