/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
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

#ifndef TEXTHANDLER_H
#define TEXTHANDLER_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>
#include "dbustypes.h"

class TextHandler : public QObject
{
    Q_OBJECT
public:
    static TextHandler *instance();
    void startChat(const QStringList &recipients, const QString &accountId);
    void startChatRoom(const QString &accountId, const QVariantMap &properties);
    void startChat(const Tp::AccountPtr &account, const Tp::Contacts &contacts);

public Q_SLOTS:
    void sendMessage(const QStringList &recipients, const QString &message, const QString &accountId);
    void sendSilentMessage(const QStringList &recipients, const QString &message, const QString &accountId);
    void sendMMS(const QStringList &recipients, const AttachmentList &attachments, const QString &accountId);
    void acknowledgeMessages(const QStringList &recipients, const QStringList &messageIds, const QString &accountId);
    void acknowledgeAllMessages(const QStringList &recipients, const QString &accountId);

protected Q_SLOTS:
    void onTextChannelAvailable(Tp::TextChannelPtr channel);
    void onContactsAvailable(Tp::PendingOperation *op);
    void onMessageSent(Tp::PendingOperation *op);
    void onConnectedChanged();

protected:
    QList<Tp::TextChannelPtr> existingChannels(const QStringList &targetIds, const QString &accountId);

private:
    explicit TextHandler(QObject *parent = 0);
    Tp::MessagePartList buildMMS(const AttachmentList &attachments);

    QList<Tp::TextChannelPtr> mChannels;
    QMap<QString, Tp::ContactPtr> mContacts;
    // keys: accountId, participants values: pending messages
    QMap<QString, QMap<QStringList, QStringList> > mPendingMessages;
    QMap<QString, QMap<QStringList, QStringList> > mPendingSilentMessages;
    QMap<QString, QMap<QStringList, QList<AttachmentList>> > mPendingMMSs;
};

#endif // TEXTHANDLER_H
