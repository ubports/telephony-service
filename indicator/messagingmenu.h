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

#ifndef MESSAGINGMENU_H
#define MESSAGINGMENU_H

#include <QObject>
#include <QMap>
#include <QDBusInterface>
#include <messaging-menu.h>
#include "accountentry.h"

class Call
{
public:
    Call() : count(0) { }
    QString number;
    int count;
    QString contactAlias;
    QUrl contactIcon;
    QString messageId;
    QString accountId;
    QDateTime timestamp;

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

    void addMessage(const QString &senderId, const QStringList &participantIds, const QString &accountId, const QString &messageId, const QDateTime &timestamp, const QString &text);
    void addFlashMessage(const QString &senderId, const QString &accountId, const QString &messageId, const QDateTime &timestamp, const QString &text);
    void removeMessage(const QString &messageId);

    void addCall(const QString &phoneNumber, const QString &accountId, const QDateTime &timestamp);
    void addCallToMessagingMenu(Call call, const QString &text);

    static void flashMessageActivateCallback(MessagingMenuMessage *message, const char *actionId, GVariant *param, MessagingMenu *instance);
    static void messageActivateCallback(MessagingMenuMessage *message, const char *actionId, GVariant *param, MessagingMenu *instance);
    static void callsActivateCallback(MessagingMenuMessage *message, const char *actionId, GVariant *param, MessagingMenu *instance);

    void showVoicemailEntry(AccountEntry *account);
    void hideVoicemailEntry(AccountEntry *account);

Q_SIGNALS:
    void replyReceived(const QStringList &recipients, const QString &accountId, const QString &reply);
    void messageRead(const QStringList &phoneNumber, const QString &accountId, const QString &messageId);

private Q_SLOTS:
    void sendMessageReply(const QString &messageId, const QString &reply);
    void showMessage(const QString &messageId);
    void callBack(const QString &messageId);
    void replyWithMessage(const QString &messageId, const QString &reply);
    void callVoicemail(const QString &messageId);
    void saveFlashMessage(const QString &messageId);

    Call callFromMessageId(const QString &messageId);

private:
    explicit MessagingMenu(QObject *parent = 0);

    MessagingMenuApp *mCallsApp;
    MessagingMenuApp *mMessagesApp;
    QMap<QString, QVariantMap> mMessages;
    QList<Call> mCalls;
    QStringList mVoicemailIds;
    int mVoicemailCount;
};

#endif // MESSAGINGMENU_H
