/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#ifndef MESSAGINGMENUMOCK_H
#define MESSAGINGMENUMOCK_H

#include <QObject>

class MessagingMenuMock : public QObject
{
    Q_OBJECT
public:
    static MessagingMenuMock *instance();


Q_SIGNALS:
    void appCreated(const QString &desktopId);
    void appRegistered(const QString &desktopId);
    void sourceAdded(const QString &app, const QString &id, const QString &icon, const QString &label);

    void messageCreated(const QString &id,
                        const QString &icon,
                        const QString &title,
                        const QString &subtitle,
                        const QString &body,
                        const QDateTime &time);
    void messageAdded(const QString &app,
                      const QString &messageId,
                      const QString &sourceId,
                      bool notify);
    void messageRemoved(const QString &app,
                        const QString &id);
    void actionAdded(const QString &messageId,
                     const QString &actionId,
                     const QString &label);

private:
    explicit MessagingMenuMock(QObject *parent = 0);

};

#endif // MESSAGINGMENUMOCK_H
