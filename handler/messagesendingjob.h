/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#ifndef MESSAGESENDINGJOB_H
#define MESSAGESENDINGJOB_H

#include <QObject>
#include <QDBusContext>
#include <TelepathyQt/Types>
#include "dbustypes.h"
#include "messagejob.h"

class AccountEntry;
class TextHandler;
class MessageSendingJobAdaptor;

struct PendingMessage {
    QString accountId;
    QString message;
    AttachmentList attachments;
    QVariantMap properties;
};
Q_DECLARE_METATYPE(PendingMessage)

class MessageSendingJob : public MessageJob, protected QDBusContext
{
    Q_OBJECT
    Q_PROPERTY(QString accountId READ accountId NOTIFY accountIdChanged)
    Q_PROPERTY(QString messageId READ messageId NOTIFY messageIdChanged)
    Q_PROPERTY(QString channelObjectPath READ channelObjectPath NOTIFY channelObjectPathChanged)
    Q_PROPERTY(QString objectPath READ objectPath CONSTANT)
    Q_PROPERTY(QVariantMap properties READ properties CONSTANT)

public:
    explicit MessageSendingJob(TextHandler *textHandler, PendingMessage message);
    ~MessageSendingJob();

    QString accountId() const;
    QString messageId() const;
    QString channelObjectPath() const;
    QString objectPath() const;
    QVariantMap properties() const;

Q_SIGNALS:
    void accountIdChanged();
    void messageIdChanged();
    void channelObjectPathChanged();

public Q_SLOTS:
    void startJob();

protected Q_SLOTS:
    void findOrCreateChannel();
    void sendMessage();

    void setAccountId(const QString &accountId);
    void setChannelObjectPath(const QString &objectPath);
    void setMessageId(const QString &id);

private:
    TextHandler *mTextHandler;
    PendingMessage mMessage;
    QString mAccountId;
    QString mMessageId;
    AccountEntry *mAccount;
    QString mChannelObjectPath;
    Tp::TextChannelPtr mTextChannel;
    QString mObjectPath;
    bool mFinished;
    MessageSendingJobAdaptor *mAdaptor;

    Tp::MessagePartList buildMessage(const PendingMessage &pendingMessage);

};

#endif // MESSAGESENDINGJOB_H
