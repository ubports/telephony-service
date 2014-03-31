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

namespace C {
#include <libintl.h>
}

#include <QDebug>
#include <libnotify/notify.h>
#include "ringtone.h"
#include "ussdindicator.h"

USSDIndicator::USSDIndicator(QObject *parent)
: QObject(parent),
  m_menuRequest(true),
  m_menuNotification(false),
  m_notifications("org.freedesktop.Notifications",
                  "/org/freedesktop/Notifications", QDBusConnection::sessionBus())
{
    connect(USSDManager::instance(), SIGNAL(notificationReceived(const QString &)), SLOT(onNotificationReceived(const QString &)));
    connect(USSDManager::instance(), SIGNAL(requestReceived(const QString &)), SLOT(onRequestReceived(const QString &)));
    connect(USSDManager::instance(), SIGNAL(initiateUSSDComplete(const QString &)), SLOT(onInitiateUSSDComplete(const QString &)));
    connect(USSDManager::instance(), SIGNAL(stateChanged(const QString &)), SLOT(onStateChanged(const QString &)));

    connect(&m_notifications, SIGNAL(ActionInvoked(uint, const QString &)), this, SLOT(actionInvoked(uint, const QString &)));
    connect(&m_notifications, SIGNAL(NotificationClosed(uint, uint)), this, SLOT(notificationClosed(uint, uint)));
}

void USSDIndicator::onNotificationReceived(const QString &message)
{
    showUSSDNotification(message, false);
}

void USSDIndicator::onRequestReceived(const QString &message)
{
    showUSSDNotification(message, true);
}

void USSDIndicator::onInitiateUSSDComplete(const QString &ussdResp)
{
    mPendingMessage = ussdResp;
}

void USSDIndicator::onStateChanged(const QString &state)
{
    if (m_notificationId == 0) {
        return;
    }

    if (state == "idle") {
        m_notifications.CloseNotification(m_notificationId);
        mPendingMessage.clear();
    }

    if (!mPendingMessage.isEmpty()) {
        showUSSDNotification(mPendingMessage, (state == "user-response"));
        mPendingMessage.clear();
    }
}

void USSDIndicator::showUSSDNotification(const QString &message, bool replyRequired)
{
    USSDMenu *menu = replyRequired ? &m_menuRequest : &m_menuNotification;
    QString actionId = "ok_id";
    QString actionLabel = C::gettext("Ok");
    if (replyRequired) {
        actionId = "reply_id";
        actionLabel = C::gettext("Reply");
    }

    // indicate to the notification-daemon, that we want to use snap-decisions
    QVariantMap notificationHints;
    notificationHints["x-canonical-snap-decisions"] = "true";
    notificationHints["x-canonical-private-button-tint"] = "true";

    QVariantMap menuModelActions;
    menuModelActions["notifications"] = menu->actionPath();

    QVariantMap menuModelPaths;
    menuModelPaths["busName"] = menu->busName();
    menuModelPaths["menuPath"] = menu->menuPath();
    menuModelPaths["actions"] = menuModelActions;

    notificationHints["x-canonical-private-menu-model"] = menuModelPaths;

    m_notificationId = m_notifications.Notify("telephony-service-indicator",
                        0, "",
                        "", message,
                        QStringList() << actionId << actionLabel << "cancel_id"
                                        << C::gettext("Cancel"), notificationHints, 0);


    Ringtone::instance()->playIncomingMessageSound();
}

void USSDIndicator::actionInvoked(uint id, const QString &actionKey) 
{
    if (id != m_notificationId) {
        return;
    }

    m_notificationId = 0;

    if (actionKey == "reply_id") {
        USSDManager::instance()->respond(m_menuRequest.response(), USSDManager::instance()->activeAccountId());
    } else {
        USSDManager::instance()->cancel(USSDManager::instance()->activeAccountId());
    }
}

void USSDIndicator::notificationClosed(uint id, uint reason) {
    if (id != m_notificationId) {
        return;
    }
    m_notifications.CloseNotification(m_notificationId);
    m_notificationId = 0;
}
