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
#include "accountentry.h"

#include <TelepathyQt/ContactManager>
#include <QDBusInterface>

typedef QMap<QString, QVariant> dbusQMap;
Q_DECLARE_METATYPE(dbusQMap)

USSDManager::USSDManager(AccountEntry *account, QObject *parent)
: QObject(parent),
  mState("idle"),
  mAccount(account)
{
    connect(mAccount, SIGNAL(connectedChanged()), this, SLOT(onConnectionChanged()));
    onConnectionChanged();
}

void USSDManager::initiate(const QString &command)
{
    QDBusInterface ussdIface(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE);
    ussdIface.asyncCall("Initiate", command);
}

void USSDManager::respond(const QString &reply)
{
    QDBusInterface ussdIface(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE);
    ussdIface.asyncCall("Respond", reply);
}

void USSDManager::cancel()
{
    QDBusInterface ussdIface(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE);
    ussdIface.asyncCall("Cancel");
}

void USSDManager::connectAllSignals()
{
    if (mBusName.isEmpty() || mObjectPath.isEmpty()) {
        return;
    }

    QDBusConnection::sessionBus().connect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "StateChanged", this, SLOT(onStateChanged(QString)));
    QDBusConnection::sessionBus().connect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "RequestReceived", this, SIGNAL(requestReceived(QString)));
    QDBusConnection::sessionBus().connect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "NotificationReceived", this, SIGNAL(notificationReceived(QString)));
    QDBusConnection::sessionBus().connect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "InitiateUSSDComplete", this, SIGNAL(initiateUSSDComplete(QString)));
    QDBusConnection::sessionBus().connect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "RespondComplete", this, SIGNAL(respondComplete(bool, QString)));
    QDBusConnection::sessionBus().connect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "BarringComplete", this, SIGNAL(barringComplete(QString, QString, QVariantMap)));
    QDBusConnection::sessionBus().connect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "ForwardingComplete", this, SIGNAL(forwardingComplete(QString, QString, QVariantMap)));
    QDBusConnection::sessionBus().connect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "WaitingComplete", this, SIGNAL(waitingComplete(QString, QVariantMap)));
    QDBusConnection::sessionBus().connect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "CallingLinePresentationComplete", this, SIGNAL(callingLinePresentationComplete(QString, QString)));
    QDBusConnection::sessionBus().connect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "CallingLineRestrictionComplete", this, SIGNAL(callingLineRestrictionComplete(QString, QString)));
    QDBusConnection::sessionBus().connect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "ConnectedLineRestrictionComplete", this, SIGNAL(connectedLineRestrictionComplete(QString, QString)));
    QDBusConnection::sessionBus().connect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "ConnectedLinePresentationComplete", this, SIGNAL(connectedLinePresentationComplete(QString, QString)));
    QDBusConnection::sessionBus().connect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "InitiateFailed", this, SIGNAL(initiateFailed()));
}

void USSDManager::disconnectAllSignals()
{
    if (mBusName.isEmpty() || mObjectPath.isEmpty()) {
        return;
    }
    QDBusConnection::sessionBus().disconnect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "StateChanged", this, SLOT(onStateChanged(QString)));
    QDBusConnection::sessionBus().disconnect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "RequestReceived", this, SIGNAL(requestReceived(QString)));
    QDBusConnection::sessionBus().disconnect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "NotificationReceived", this, SIGNAL(notificationReceived(QString)));
    QDBusConnection::sessionBus().disconnect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "InitiateUSSDComplete", this, SIGNAL(initiateUSSDComplete(QString)));
    QDBusConnection::sessionBus().disconnect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "RespondComplete", this, SIGNAL(respondComplete(bool, QString)));
    QDBusConnection::sessionBus().disconnect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "BarringComplete", this, SIGNAL(barringComplete(QString, QString, QVariantMap)));
    QDBusConnection::sessionBus().disconnect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "ForwardingComplete", this, SIGNAL(forwardingComplete(QString, QString, QVariantMap)));
    QDBusConnection::sessionBus().disconnect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "WaitingComplete", this, SIGNAL(waitingComplete(QString, QVariantMap)));
    QDBusConnection::sessionBus().disconnect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "CallingLinePresentationComplete", this, SIGNAL(callingLinePresentationComplete(QString, QString)));
    QDBusConnection::sessionBus().disconnect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "CallingLineRestrictionComplete", this, SIGNAL(callingLineRestrictionComplete(QString, QString)));
    QDBusConnection::sessionBus().disconnect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "ConnectedLineRestrictionComplete", this, SIGNAL(connectedLineRestrictionComplete(QString, QString)));
    QDBusConnection::sessionBus().disconnect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "ConnectedLinePresentationComplete", this, SIGNAL(connectedLinePresentationComplete(QString, QString)));
    QDBusConnection::sessionBus().disconnect(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE, "InitiateFailed", this, SIGNAL(initiateFailed()));
}

void USSDManager::onConnectionChanged()
{
    disconnectAllSignals();

    if (mAccount->account()->connection().isNull()) {
        qDebug() << "USSDManager: Failed to connect signals";
        return;
    }

    mBusName = mAccount->account()->connection()->busName();
    mObjectPath = mAccount->account()->connection()->objectPath();

    QDBusInterface ussdIface(mBusName, mObjectPath, CANONICAL_TELEPHONY_USSD_IFACE);
    mState = ussdIface.property("State").toString();

    connectAllSignals();
}

void USSDManager::onStateChanged(const QString &state)
{
    mState = state;
    Q_EMIT stateChanged(mState);
    Q_EMIT activeChanged();
}

bool USSDManager::active() const
{
    return mState != "idle";
}

QString USSDManager::state() const
{
    return mState;
}
