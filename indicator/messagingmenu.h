/*
 * Copyright (C) 2012-2017 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrman@canonical.com>
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
#include <libnotify/notify.h>

class Call
{
public:
    Call() : count(0) { }
    QString targetId;
    int count;
    QString contactAlias;
    QUrl contactIcon;
    QString messageId;
    QString accountId;
    QDateTime timestamp;

    bool operator==(const Call &other) {
        return other.targetId == targetId;
    }
};

class TextChannelObserver;

class NotificationData {
public:
    NotificationData() : targetType(0), observer(NULL), notificationList(NULL) {}
    QString accountId;
    QString senderId;
    QString targetId;
    uint targetType;
    QStringList participantIds;
    QDateTime timestamp;
    QString messageText;
    QString encodedEventId;
    QString alias;
    QString roomName;
    QString icon;
    QString notificationTitle;
    QString messageReply;
    TextChannelObserver *observer;
    QMap<NotifyNotification*, NotificationData*> *notificationList;
};

class MessagingMenu : public QObject
{
    Q_OBJECT
public:
    static MessagingMenu *instance();
    virtual ~MessagingMenu();

    void addMessage(NotificationData notificationData);
    void addFlashMessage(NotificationData notificationData);
    void removeMessage(const QString &messageId);
    void addNotification(NotificationData notificationData);

    void addCall(const QString &targetId, const QString &accountId, const QDateTime &timestamp);
    void removeCall(const QString &targetId, const QString &accountId);
    void addCallToMessagingMenu(Call call, const QString &text, bool supportsTextReply = true);

    static void flashMessageActivateCallback(MessagingMenuMessage *message, const char *actionId, GVariant *param, MessagingMenu *instance);
    static void messageActivateCallback(MessagingMenuMessage *message, const char *actionId, GVariant *param, MessagingMenu *instance);
    static void callsActivateCallback(MessagingMenuMessage *message, const char *actionId, GVariant *param, MessagingMenu *instance);

    void showVoicemailEntry(AccountEntry *account);
    void hideVoicemailEntry(AccountEntry *account);

Q_SIGNALS:
    void replyReceived(NotificationData notificationData);
    void messageRead(NotificationData notificationData);

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
    QMap<QString, NotificationData> mMessages;
    QList<Call> mCalls;
    QStringList mVoicemailIds;
    int mVoicemailCount;
};

#endif // MESSAGINGMENU_H
