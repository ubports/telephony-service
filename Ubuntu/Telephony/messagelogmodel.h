/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MESSAGELOGMODEL_H
#define MESSAGELOGMODEL_H

#include "conversationfeedmodel.h"
#include "conversationfeeditem.h"

class MessageLogEntry : public ConversationFeedItem {
    Q_OBJECT
    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged)
    Q_PROPERTY(QString messageId READ messageId WRITE setMessageId NOTIFY messageIdChanged)
    Q_PROPERTY(QString date READ date)
public:
    explicit MessageLogEntry(QObject *parent = 0) : ConversationFeedItem(parent) { }
    void setMessage(const QString &message) { mMessage = message; Q_EMIT messageChanged(); }
    QString message() { return mMessage; }

    void setMessageId(const QString &messageId) { mMessageId = messageId; Q_EMIT messageIdChanged(); }
    QString messageId() { return mMessageId; }

    QString date() { return timestamp().date().toString(Qt::DefaultLocaleLongDate); }

Q_SIGNALS:
    void messageChanged();
    void messageIdChanged();

public:
    QString mMessage;
    QString mMessageId;

};

class MessageLogModel : public ConversationFeedModel
{
    Q_OBJECT
public:
    explicit MessageLogModel(QObject *parent = 0);


    QString itemType(const QModelIndex &index) const;
    bool matchesSearch(const QString &searchTerm, const QModelIndex &index) const;

Q_SIGNALS:
    void messageRead(const QString &number, const QString &messageId);

public Q_SLOTS:
    void onMessageReceived(const QString &number, const QString &message, const QDateTime &timestamp, const QString &messageId, bool unread);
    void onMessageSent(const QString &number, const QString &message);
    void appendMessage(const QString &number,
                       const QString &message,
                       bool incoming,
                       const QDateTime &timestamp = QDateTime::currentDateTime(),
                       const QString &messageId = QString::null,
                       bool unread = false);


protected Q_SLOTS:
    void onNewItemChanged();

protected:
    MessageLogEntry *messageById(const QString &messageId);

};

#endif // MESSAGELOGMODEL_H
