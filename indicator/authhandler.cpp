/*
 * Copyright (C) 2015 Canonical, Ltd.
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
#include "applicationutils.h"
#include "authhandler.h"
#include "telepathyhelper.h"

// FIXME: port to libqmenumodel if possible
AuthHandler::AuthHandler(QObject *parent)
: QObject(parent),
  m_menuNotification("authentication", false),
  m_notifications("org.freedesktop.Notifications",
                  "/org/freedesktop/Notifications", QDBusConnection::sessionBus())
{
    setupAccounts();
    connect(TelepathyHelper::instance(), SIGNAL(setupReady()), this, SLOT(setupAccounts()));
    connect(TelepathyHelper::instance(), SIGNAL(accountsChanged()), this, SLOT(setupAccounts()));
    connect(&m_notifications, SIGNAL(ActionInvoked(uint, const QString &)), this, SLOT(actionInvoked(uint, const QString &)));
    connect(&m_notifications, SIGNAL(NotificationClosed(uint, uint)), this, SLOT(notificationClosed(uint, uint)));

    connect(TelepathyHelper::instance(), &TelepathyHelper::setupReady, [&]() {
        Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->accounts()) {
            processStatusChange(account, account->account()->connectionStatus());
        }
    });
}

void AuthHandler::setupAccounts()
{
    Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->accounts()) {
        connect(account, SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)), SLOT(onConnectionStatusChanged(Tp::ConnectionStatus)), Qt::UniqueConnection);
    }
}

void AuthHandler::onConnectionStatusChanged(Tp::ConnectionStatus status)
{
    AccountEntry *account = qobject_cast<AccountEntry*>(sender());
    if (!account) {
        return;
    }

    processStatusChange(account, status);
}

void AuthHandler::processStatusChange(AccountEntry *account, Tp::ConnectionStatus status)
{    
    QString title;
    QString message = QString::fromUtf8(C::gettext("Authentication failed. Do you want to review your credentials?"));

    if (status == Tp::ConnectionStatusDisconnected && 
            account->account()->connectionStatusReason() == Tp::ConnectionStatusReasonAuthenticationFailed &&
            !mIgnoredAccounts.contains(account->accountId())) {
        QString serviceAndDisplayName;
        QString displayName = account->displayName();
        if (displayName.isEmpty()) {
            serviceAndDisplayName = account->account()->serviceName();
        } else {
            serviceAndDisplayName = QString("%1: %2").arg(account->account()->serviceName()).arg(displayName);
        }
        title = serviceAndDisplayName;
    } else {
        return;
    }

    NotificationMenu *menu = &m_menuNotification;
    QString yesActionId = "yes_id";
    QString yesActionLabel = C::gettext("Yes");

    QString noActionId = "no_id";
    QString noActionLabel = C::gettext("No");

    QStringList actions;
    actions << yesActionId << yesActionLabel << noActionId << noActionLabel;

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

    uint notificationId = m_notifications.Notify("telephony-service-indicator",
                        0, "", title, message, actions, notificationHints, 0);
    mAuthFailureRequests[notificationId] = account;
}

void AuthHandler::actionInvoked(uint id, const QString &actionKey) 
{
    if (!mAuthFailureRequests.keys().contains(id)) {
        return;
    }

    if (actionKey == "yes_id") {
        ApplicationUtils::openUrl(QString("settings:///system/online-accounts"));
    } else {
        mIgnoredAccounts << mAuthFailureRequests[id]->accountId();
    }
}

void AuthHandler::notificationClosed(uint id, uint reason) {
    if (!mAuthFailureRequests.keys().contains(id)) {
        return;
    }
    m_notifications.CloseNotification(id);
    mAuthFailureRequests.remove(id);
}

void AuthHandler::clear()
{
    Q_FOREACH (uint id, mAuthFailureRequests.keys()) {
        mAuthFailureRequests.remove(id);
        m_notifications.CloseNotification(id);
    }
}
