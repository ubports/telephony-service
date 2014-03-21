/*
 * Copyright (C) 2012 Canonical, Ltd.
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

#include "ussdmanager.h"
#include "telepathyhelper.h"

#include <TelepathyQt/ContactManager>
#include <QDBusInterface>

typedef QMap<QString, QVariant> dbusQMap;
Q_DECLARE_METATYPE(dbusQMap)

USSDManager *USSDManager::instance()
{
    static USSDManager *self = new USSDManager();
    return self;
}

USSDManager::USSDManager(QObject *parent)
: QObject(parent), mActive(false)
{
    connect(TelepathyHelper::instance(), SIGNAL(connectedChanged()), SLOT(onConnectedChanged()));
}

USSDManager::initiate(const QString &command, const QString &accountId)
{
    Tp::AccountPtr account;
    if (accountId.isNull()) {
        account = TelepathyHelper::instance()->accounts()[0];
    } else {
        account = TelepathyHelper::instance()->accountForId(accountId);
    }

    Tp::ConnectionPtr conn(account->connection());
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    QDBusInterface connIface(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE);
    connIface.asyncCall("Initiate");
}

USSDManager::respond(const QString &reply, const QString &accountId)
{
    Tp::AccountPtr account;
    if (accountId.isNull()) {
        account = TelepathyHelper::instance()->accounts()[0];
    } else {
        account = TelepathyHelper::instance()->accountForId(accountId);
    }

    Tp::ConnectionPtr conn(account->connection());
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    QDBusInterface connIface(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE);
    connIface.asyncCall("Respond", reply);
}

USSDManager::cancel(const QString &accountId)
{
    Tp::AccountPtr account;
    if (accountId.isNull()) {
        account = TelepathyHelper::instance()->accounts()[0];
    } else {
        account = TelepathyHelper::instance()->accountForId(accountId);
    }

    Tp::ConnectionPtr conn(account->connection());
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    QDBusInterface connIface(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE);
    connIface.asyncCall("Cancel");
}

void USSDManager::onConnectedChanged()
{
    // everytime the connection changes we need to check if there is any ongoing ussd session
    mActive = false;
    mActiveAccountId = QString::null;
    mState = QString("idle");

    if (!TelepathyHelper::instance()->connected()) {
        Q_EMIT stateChanged();
        Q_EMIT ativeChanged();
        Q_EMIT activeAccountIdChanged();
        return;
    }

    Q_FOREACH(const Tp::AccountPtr &account, TelepathyHelper::instance()->accounts()) {
        // disconnect all and reconnect only the online accounts
        Tp::ConnectionPtr conn(account->connection());
        QString busName = conn->busName();
        QString objectPath = conn->objectPath();

        QDBusConnection::sessionBus().disconnect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "StateChanged", this, SLOT(onStateChanged(QString))
        if (TelepathyHelper::instance()->isAccountConnected(account)) {
            QDBusConnection::sessionBus().connect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "StateChanged", this, SLOT(onStateChanged(QString))
            QDBusInterface ussdIface(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE);
            mState = ussdIface.property("State");
            if (active()) {
                mActiveAccountId = account->uniqueIdentifier();
            }
        }
    }
    Q_EMIT stateChanged();
    Q_EMIT ativeChanged();
    Q_EMIT activeAccountIdChanged();
}

bool USSDManager::active() const
{
    return mState != "idle" && !mState.isEmpty();
}

QString USSDManager::activeAccountId() const
{
    return mActiveAccountId;
}

