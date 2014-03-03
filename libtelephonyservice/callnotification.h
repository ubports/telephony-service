/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#ifndef CALLNOTIFICATION_H
#define CALLNOTIFICATION_H

#include <QObject>

class CallNotification : public QObject
{
    Q_OBJECT
public:
    enum NotificationReason {
        CallHeld,
        CallEnded,
        CallRejected
    };

public Q_SLOTS:
    static CallNotification *instance();
    void showNotificationForCall(const QStringList &participants, NotificationReason reason);

private:
    explicit CallNotification(QObject *parent = 0);
};

#endif // CALLNOTIFICATION_H
