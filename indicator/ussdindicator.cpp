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
#include "ofonoaccountentry.h"
#include "telepathyhelper.h"

USSDIndicator::USSDIndicator(QObject *parent)
: QObject(parent),
  m_menuRequest("ussd", true),
  m_menuNotification("ussd", false),
  m_notifications("org.freedesktop.Notifications",
                  "/org/freedesktop/Notifications", QDBusConnection::sessionBus())
{
    setupAccounts();
    connect(TelepathyHelper::instance(), SIGNAL(accountsChanged()), this, SLOT(setupAccounts()));
    connect(&m_notifications, SIGNAL(ActionInvoked(uint, const QString &)), this, SLOT(actionInvoked(uint, const QString &)));
    connect(&m_notifications, SIGNAL(NotificationClosed(uint, uint)), this, SLOT(notificationClosed(uint, uint)));
}

void USSDIndicator::setupAccounts()
{
    Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->accounts()) {
        OfonoAccountEntry *ofonoAccount = qobject_cast<OfonoAccountEntry*>(account);
        if (!ofonoAccount) {
            continue;
        }
        
        connect(ofonoAccount->ussdManager(), SIGNAL(notificationReceived(const QString &)), SLOT(onNotificationReceived(const QString &)), Qt::UniqueConnection);
        connect(ofonoAccount->ussdManager(), SIGNAL(requestReceived(const QString &)), SLOT(onRequestReceived(const QString &)), Qt::UniqueConnection);
        connect(ofonoAccount->ussdManager(), SIGNAL(initiateUSSDComplete(const QString &)), SLOT(onInitiateUSSDComplete(const QString &)), Qt::UniqueConnection);
        connect(ofonoAccount->ussdManager(), SIGNAL(respondComplete(bool, const QString &)), SLOT(onRespondComplete(bool, const QString &)), Qt::UniqueConnection);
        connect(ofonoAccount->ussdManager(), SIGNAL(stateChanged(const QString &)), SLOT(onStateChanged(const QString &)), Qt::UniqueConnection);
    }
}

void USSDIndicator::onNotificationReceived(const QString &message)
{
    USSDManager *ussdManager = qobject_cast<USSDManager*>(sender());
    if (!ussdManager) {
        return;
    }
    showUSSDNotification(message, false, ussdManager);
}

void USSDIndicator::onRequestReceived(const QString &message)
{
    USSDManager *ussdManager = qobject_cast<USSDManager*>(sender());
    if (!ussdManager) {
        return;
    }
    showUSSDNotification(message, true, ussdManager);
}

void USSDIndicator::onInitiateUSSDComplete(const QString &ussdResp)
{
    USSDManager *ussdManager = qobject_cast<USSDManager*>(sender());
    if (!ussdManager) {
        return;
    }
    showUSSDNotification(ussdResp, (ussdManager->state() == "user-response"), ussdManager);
}

void USSDIndicator::onRespondComplete(bool success, const QString &ussdResp)
{
    USSDManager *ussdManager = qobject_cast<USSDManager*>(sender());
    if (!ussdManager) {
        return;
    }
    if (success) {
        showUSSDNotification(ussdResp, (ussdManager->state() == "user-response"), ussdManager);
    }
}

void USSDIndicator::onStateChanged(const QString &state)
{
    // TODO: check if we should close notifications when the state is idle
}

void USSDIndicator::showUSSDNotification(const QString &message, bool replyRequired, USSDManager *ussdManager)
{
    NotificationMenu *menu = replyRequired ? &m_menuRequest : &m_menuNotification;
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
    mUSSDRequests[m_notificationId] = ussdManager;

    Ringtone::instance()->playIncomingMessageSound();
}

void USSDIndicator::actionInvoked(uint id, const QString &actionKey) 
{
    if (id != m_notificationId) {
        return;
    }

    USSDManager *ussdManager = mUSSDRequests.take(id);
    if (!ussdManager) {
        return;
    }

    m_notificationId = 0;

    if (actionKey == "reply_id") {
        ussdManager->respond(m_menuRequest.response());
    } else if (actionKey == "cancel_id") {
        ussdManager->cancel();
    }
    m_menuRequest.clearResponse();
}

void USSDIndicator::notificationClosed(uint id, uint reason) {
    if (id != m_notificationId) {
        return;
    }
    m_notifications.CloseNotification(m_notificationId);
    m_notificationId = 0;
}

void USSDIndicator::clear()
{
    if (m_notificationId != 0) {
        USSDManager *ussdManager = mUSSDRequests.take(m_notificationId);
        if (ussdManager) {
            ussdManager->cancel();
        }

        m_notifications.CloseNotification(m_notificationId);
        m_notificationId = 0;
    }
}
