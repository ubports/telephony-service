/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#ifndef CONTACTWATCHER_H
#define CONTACTWATCHER_H

#include <QObject>
#include <QContactManager>
#include <QContactAbstractRequest>

QTCONTACTS_USE_NAMESPACE

class ContactWatcher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString contactId READ contactId NOTIFY contactIdChanged)
    Q_PROPERTY(QString avatar READ avatar NOTIFY avatarChanged)
    Q_PROPERTY(QString alias READ alias NOTIFY aliasChanged)
    Q_PROPERTY(QString phoneNumber READ phoneNumber WRITE setPhoneNumber NOTIFY phoneNumberChanged)
    Q_PROPERTY(bool isUnknown READ isUnknown NOTIFY isUnknownChanged)
public:
    explicit ContactWatcher(QObject *parent = 0);

    static QContactManager *engineInstance();

    QString contactId() const;
    QString avatar() const;
    QString alias() const;
    QString phoneNumber() const;
    void setPhoneNumber(const QString &phoneNumber);
    bool isUnknown() const;
    
Q_SIGNALS:
    void contactIdChanged();
    void avatarChanged();
    void aliasChanged();
    void phoneNumberChanged();
    void isUnknownChanged();

protected Q_SLOTS:
    void onContactsAdded(QList<QContactId> ids);
    void onContactsChanged(QList<QContactId> ids);
    void onContactsRemoved(QList<QContactId> ids);
    void onRequestStateChanged(QContactAbstractRequest::State state);
    void resultsAvailable();

private:
    void searchByPhoneNumber(const QString &phoneNumber);
    QString mContactId;
    QString mAvatar;
    QString mAlias;
    QString mPhoneNumber;
};

#endif // CONTACTWATCHER_H
