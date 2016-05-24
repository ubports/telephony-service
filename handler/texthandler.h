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
#include "messagesendingjob.h"

class TextHandler : public QObject
{
    Q_OBJECT
public:
    static TextHandler *instance();
    void startChat(const QString &accountId, const QVariantMap &properties);

    friend class MessageSendingJob;

public Q_SLOTS:
    QString sendMessage(const QString &accountId, const QString &message, const AttachmentList &attachments, const QVariantMap &properties);
    void acknowledgeMessages(const QStringList &recipients, const QStringList &messageIds, const QString &accountId);
    void acknowledgeAllMessages(const QStringList &recipients, const QString &accountId);

protected Q_SLOTS:
    void onTextChannelAvailable(Tp::TextChannelPtr channel);
    void onTextChannelInvalidated();
    void onConnectedChanged();

protected:
    QList<Tp::TextChannelPtr> existingChannels(const QString &accountId, const QVariantMap &properties);

private:
    explicit TextHandler(QObject *parent = 0);


    QList<Tp::TextChannelPtr> mChannels;
    QList<PendingMessage> mPendingMessages;
};

#endif // TEXTHANDLER_H
