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

#ifndef CONVERSATIONFEEDITEM_H
#define CONVERSATIONFEEDITEM_H

#include <QDateTime>
#include <QObject>
#include <QUrl>

class ConversationFeedItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString contactId READ contactId WRITE setContactId NOTIFY contactIdChanged)
    Q_PROPERTY(QString contactAlias READ contactAlias WRITE setContactAlias NOTIFY contactAliasChanged)
    Q_PROPERTY(QUrl contactAvatar READ contactAvatar WRITE setContactAvatar NOTIFY contactAvatarChanged)
    Q_PROPERTY(bool incoming READ incoming WRITE setIncoming NOTIFY incomingChanged)
    Q_PROPERTY(bool newItem READ newItem WRITE setNewItem NOTIFY newItemChanged)
    Q_PROPERTY(QDateTime timestamp READ timestamp WRITE setTimestamp NOTIFY timestampChanged)
    Q_PROPERTY(QString phoneNumber READ phoneNumber WRITE setPhoneNumber NOTIFY phoneNumberChanged)

public:
    explicit ConversationFeedItem(QObject *parent = 0);
    virtual ~ConversationFeedItem();
    
    QString contactId() const;
    void setContactId(const QString &value);

    QString contactAlias() const;
    void setContactAlias(const QString &value);

    QUrl contactAvatar() const;
    void setContactAvatar(const QUrl &value);

    bool incoming() const;
    void setIncoming(bool value);

    bool newItem() const;
    void setNewItem(bool value);

    QDateTime timestamp() const;
    void setTimestamp(const QDateTime &value);

    void setPhoneNumber(const QString &phone);
    QString phoneNumber();

Q_SIGNALS:
    void contactIdChanged();
    void contactAliasChanged();
    void contactAvatarChanged();
    void incomingChanged();
    void newItemChanged();
    void timestampChanged();
    void phoneNumberChanged();

private:
    QString mContactId;
    QString mContactAlias;
    QUrl mContactAvatar;
    bool mIncoming;
    bool mNewItem;
    QDateTime mTimestamp;
    QString mPhoneNumber;
};

#endif // CONVERSATIONFEEDITEM_H
