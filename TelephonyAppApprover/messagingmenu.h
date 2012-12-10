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

#ifndef MESSAGINGMENU_H
#define MESSAGINGMENU_H

#include <QObject>
#include <QMap>
#include <QDBusInterface>
#include <messaging-menu.h>
#include <messaging-menu-message.h>

class Call
{
public:
    Call() : count(0) { }
    QString number;
    int count;
    QString contactAlias;
    QString contactIcon;
    QString messageId;

    bool operator==(const Call &other) {
        return other.number == number;
    }
};

class MessagingMenu : public QObject
{
    Q_OBJECT
public:
    static MessagingMenu *instance();
    virtual ~MessagingMenu();

    void addMessage(const QString &phoneNumber, const QString &messageId, const QDateTime &timestamp, const QString &text);
    void removeMessage(const QString &messageId);

    void addCall(const QString &phoneNumber, const QDateTime &timestamp);

    static void messageActivateCallback(MessagingMenuMessage *message, const char *actionId, GVariant *param, MessagingMenu *instance);
    static void callsActivateCallback(MessagingMenuMessage *message, const char *actionId, GVariant *param, MessagingMenu *instance);

    void showVoicemailEntry(int count);
    void hideVoicemailEntry();

private Q_SLOTS:
    void sendMessageReply(const QString &messageId, const QString &reply);
    void showMessage(const QString &messageId);
    void callBack(const QString &messageId);
    void replyWithMessage(const QString &messageId, const QString &reply);
    void callVoicemail(const QString &messageId);

    Call callFromMessageId(const QString &messageId);

private:
    explicit MessagingMenu(QObject *parent = 0);

    MessagingMenuApp *mCallsApp;
    MessagingMenuApp *mMessagesApp;
    QMap<QString, QString> mMessages;
    QList<Call> mCalls;
    QString mVoicemailId;
    int mVoicemailCount;
    QDBusInterface mTelephonyAppInterface;
};

#endif // MESSAGINGMENU_H
