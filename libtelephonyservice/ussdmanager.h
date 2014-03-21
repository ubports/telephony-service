/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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

#ifndef USSDMANAGER_H
#define USSDMANAGER_H

#include <QtCore/QMap>
#include <QDBusInterface>

class TelepathyHelper;

class USSDManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active 
               READ active
               NOTIFY activeChanged)
    Q_PROPERTY(bool activeAccountId
               READ activeAccountId
               NOTIFY activeAccountIdChanged)
public:
    static USSDManager *instance();
    Q_INVOKABLE void initiate(const QString &command, const QString &accountId = QString::null);
    Q_INVOKABLE void respond(const QString &reply, const QString &accountId = QString::null);
    Q_INVOKABLE void cancel(const QString &accountId = QString::null);

    bool active() const;
    QString activeAccountId() const;

public Q_SLOTS:
    void onConnectedChanged();

Q_SIGNALS:
    void activeChanged();
    void activeAccountIdChanged();

private:
    explicit USSDManager(QObject *parent = 0);
    bool mActive;
    QString mActiveAccountId;
};

#endif // USSDMANAGER_H
