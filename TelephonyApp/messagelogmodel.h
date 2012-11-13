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

#ifndef MESSAGELOGMODEL_H
#define MESSAGELOGMODEL_H

#include "abstractloggermodel.h"

class MessageLogEntry : public LoggerItem {
    Q_OBJECT
    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged)
    Q_PROPERTY(QString messageId READ messageId WRITE setMessageId NOTIFY messageIdChanged)
    Q_PROPERTY(QString date READ date)
    Q_PROPERTY(bool isLatest READ isLatest WRITE setIsLatest NOTIFY isLatestChanged)
public:
    explicit MessageLogEntry(QObject *parent = 0) : LoggerItem(parent) { }
    void setMessage(const QString &message) { mMessage = message; Q_EMIT messageChanged(); }
    QString message() { return mMessage; }

    void setMessageId(const QString &messageId) { mMessageId = messageId; Q_EMIT messageIdChanged(); }
    QString messageId() { return mMessageId; }

    void setIsLatest(bool isLatest) { mIsLatest = isLatest; Q_EMIT isLatestChanged(); }
    bool isLatest() { return mIsLatest; }

    QString date() { return timestamp().date().toString(Qt::DefaultLocaleLongDate); }
Q_SIGNALS:
    void messageChanged();
    void messageIdChanged();
    void isLatestChanged();

public:
    QString mMessage;
    QString mMessageId;
    bool mIsLatest;
};

class MessageLogModel : public AbstractLoggerModel
{
    Q_OBJECT
public:
    explicit MessageLogModel(QObject *parent = 0);

    Q_INVOKABLE void appendMessage(const QString &number,
                                   const QString &message,
                                   bool incoming,
                                   const QDateTime &timestamp = QDateTime::currentDateTime(),
                                   const QString &messageId = QString::null);

    QString itemType(const QModelIndex &index) const;
    bool matchesSearch(const QString &searchTerm, const QModelIndex &index) const;

public Q_SLOTS:
    void populate();
    void onMessageReceived(const QString &number, const QString &message, const QDateTime &timestamp, const QString &messageId);
    void onMessageSent(const QString &number, const QString &message);

protected:
    MessageLogEntry *createEntry(const Tpl::EventPtr &event);
    void handleEvents(const Tpl::EventPtrList &events);
    void updateLatestMessages(const QString &phoneNumber);
    MessageLogEntry *messageById(const QString &messageId);

};

#endif // MESSAGELOGMODEL_H
