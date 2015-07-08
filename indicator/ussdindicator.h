/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#ifndef USSDINDICATOR_H
#define USSDINDICATOR_H

#include <QDBusInterface>
#include <QVariantMap>
#include "indicator/NotificationsInterface.h"
#include "ussdmanager.h"
#include "ussdmenu.h"
#include "ofonoaccountentry.h"

class USSDIndicator : public QObject
{
    Q_OBJECT
public:
    explicit USSDIndicator(QObject *parent = 0);
    void showUSSDNotification(const QString &message, bool replyRequired, USSDManager *ussdManager);

public Q_SLOTS:
    void onNotificationReceived(const QString &message);
    void onRequestReceived(const QString &message);
    void onInitiateUSSDComplete(const QString &ussdResp);
    void onRespondComplete(bool success, const QString &ussdResp);
    void onStateChanged(const QString &state);
    void actionInvoked(uint id, const QString &actionKey);
    void notificationClosed(uint id, uint reason);
    void clear();
private Q_SLOTS:
    void setupAccounts();

private:
    unsigned int m_notificationId;
    USSDMenu m_menuRequest;
    USSDMenu m_menuNotification;
    QString mPendingMessage;
    org::freedesktop::Notifications m_notifications;
    QMap<int, USSDManager*> mUSSDRequests;
    QList<OfonoAccountEntry*> mAccounts;
};

#endif // USSDINDICATOR_H
