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

#ifndef CHATSTARTINGJOB_H
#define CHATSTARTINGJOB_H

#include <QObject>
#include "messagejob.h"
#include <TelepathyQt/Types>
#include <TelepathyQt/PendingOperation>

class TextHandler;

class ChatStartingJob : public MessageJob
{
    Q_OBJECT
    Q_PROPERTY(QString accountId READ accountId CONSTANT)
    Q_PROPERTY(Tp::TextChannelPtr textChannel READ textChannel NOTIFY textChannelChanged)
    Q_PROPERTY(QString channelObjectPath READ channelObjectPath NOTIFY channelObjectPathChanged)
public:
    ChatStartingJob(TextHandler *textHandler, const QString &accountId, const QVariantMap &properties);

    QString accountId();
    Tp::TextChannelPtr textChannel() const;
    QString channelObjectPath() const;

public Q_SLOTS:
    virtual void startJob();

Q_SIGNALS:
    void textChannelChanged();
    void channelObjectPathChanged();


protected Q_SLOTS:
    void startTextChat(const Tp::AccountPtr &account, const QVariantMap &properties);
    void startTextChatRoom(const Tp::AccountPtr &account, const QVariantMap &properties);
    void setTextChannel(Tp::TextChannelPtr channel);

    void onChannelRequestFinished(Tp::PendingOperation *op);

private:
    TextHandler *mTextHandler;
    QString mAccountId;
    QVariantMap mProperties;
    Tp::TextChannelPtr mTextChannel;

};

#endif // CHATSTARTINGJOB_H
