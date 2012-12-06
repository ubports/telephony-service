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
#include <messaging-menu.h>
#include <messaging-menu-message.h>

class Call
{
public:
    Call() : count(0), message(0) { }
    QString number;
    int count;
    QString contactAlias;
    QString contactIcon;
    MessagingMenuMessage *message;

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

    void showVoicemailEntry(int count);
    void hideVoicemailEntry();

private:
    explicit MessagingMenu(QObject *parent = 0);

    MessagingMenuApp *mApp;
    QMap<QString, MessagingMenuMessage*> mMessages;
    QList<Call> mCalls;
};

#endif // MESSAGINGMENU_H
