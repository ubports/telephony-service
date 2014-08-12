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

USSDManager *USSDManager::instance()
{
    static USSDManager *self = new USSDManager();
    return self;
}

USSDManager::USSDManager(QObject *parent)
: QObject(parent)
{
    connect(TelepathyHelper::instance(), SIGNAL(accountsChanged()), SLOT(onAccountsChanged()));
    onAccountsChanged();
}

Tp::ConnectionPtr USSDManager::connectionForAccountId(const QString &accountId)
{
    AccountEntry *accountEntry;
    if (accountId.isNull()) {
        accountEntry = TelepathyHelper::instance()->accounts()[0];
    } else {
        accountEntry = TelepathyHelper::instance()->accountForId(accountId);
    }

    return accountEntry->account()->connection();
}

void USSDManager::initiate(const QString &command, const QString &accountId)
{
    Tp::ConnectionPtr conn = connectionForAccountId(accountId);
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    QDBusInterface ussdIface(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE);
    ussdIface.asyncCall("Initiate", command);
}

void USSDManager::respond(const QString &reply, const QString &accountId)
{
    Tp::ConnectionPtr conn = connectionForAccountId(accountId);
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    QDBusInterface ussdIface(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE);
    ussdIface.asyncCall("Respond", reply);
}

void USSDManager::cancel(const QString &accountId)
{
    Tp::ConnectionPtr conn = connectionForAccountId(accountId);
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    QDBusInterface ussdIface(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE);
    ussdIface.asyncCall("Cancel");
}

void USSDManager::disconnectAllSignals(const Tp::ConnectionPtr& conn)
{
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();

    QDBusConnection::sessionBus().disconnect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "StateChanged", this, SLOT(onStateChanged(QString)));
    QDBusConnection::sessionBus().disconnect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "RequestReceived", this, SIGNAL(requestReceived(QString)));
    QDBusConnection::sessionBus().disconnect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "NotificationReceived", this, SIGNAL(notificationReceived(QString)));
    QDBusConnection::sessionBus().disconnect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "InitiateUSSDComplete", this, SIGNAL(initiateUSSDComplete(QString)));
    QDBusConnection::sessionBus().disconnect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "RespondComplete", this, SIGNAL(respondComplete(bool, QString)));
    QDBusConnection::sessionBus().disconnect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "BarringComplete", this, SIGNAL(barringComplete(QString, QString, QVariantMap)));
    QDBusConnection::sessionBus().disconnect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "ForwardingComplete", this, SIGNAL(forwardingComplete(QString, QString, QVariantMap)));
    QDBusConnection::sessionBus().disconnect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "WaitingComplete", this, SIGNAL(waitingComplete(QString, QVariantMap)));
    QDBusConnection::sessionBus().disconnect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "CallingLinePresentationComplete", this, SIGNAL(callingLinePresentationComplete(QString, QString)));
    QDBusConnection::sessionBus().disconnect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "CallingLineRestrictionComplete", this, SIGNAL(callingLineRestrictionComplete(QString, QString)));
    QDBusConnection::sessionBus().disconnect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "ConnectedLineRestrictionComplete", this, SIGNAL(connectedLineRestrictionComplete(QString, QString)));
    QDBusConnection::sessionBus().disconnect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "ConnectedLinePresentationComplete", this, SIGNAL(connectedLinePresentationComplete(QString, QString)));
    QDBusConnection::sessionBus().disconnect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "InitiateFailed", this, SIGNAL(initiateFailed()));
}

void USSDManager::connectAllSignals(const Tp::ConnectionPtr& conn)
{
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    qDebug() << "USSD "<<  objectPath;

    QDBusConnection::sessionBus().connect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "StateChanged", this, SLOT(onStateChanged(QString)));
    QDBusConnection::sessionBus().connect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "RequestReceived", this, SIGNAL(requestReceived(QString)));
    QDBusConnection::sessionBus().connect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "NotificationReceived", this, SIGNAL(notificationReceived(QString)));
    QDBusConnection::sessionBus().connect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "InitiateUSSDComplete", this, SIGNAL(initiateUSSDComplete(QString)));
    QDBusConnection::sessionBus().connect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "RespondComplete", this, SIGNAL(respondComplete(bool, QString)));
    QDBusConnection::sessionBus().connect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "BarringComplete", this, SIGNAL(barringComplete(QString, QString, QVariantMap)));
    QDBusConnection::sessionBus().connect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "ForwardingComplete", this, SIGNAL(forwardingComplete(QString, QString, QVariantMap)));
    QDBusConnection::sessionBus().connect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "WaitingComplete", this, SIGNAL(waitingComplete(QString, QVariantMap)));
    QDBusConnection::sessionBus().connect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "CallingLinePresentationComplete", this, SIGNAL(callingLinePresentationComplete(QString, QString)));
    QDBusConnection::sessionBus().connect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "CallingLineRestrictionComplete", this, SIGNAL(callingLineRestrictionComplete(QString, QString)));
    QDBusConnection::sessionBus().connect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "ConnectedLineRestrictionComplete", this, SIGNAL(connectedLineRestrictionComplete(QString, QString)));
    QDBusConnection::sessionBus().connect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "ConnectedLinePresentationComplete", this, SIGNAL(connectedLinePresentationComplete(QString, QString)));
    QDBusConnection::sessionBus().connect(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE, "InitiateFailed", this, SIGNAL(initiateFailed()));
}

void USSDManager::accountConnectedChanged()
{
    AccountEntry *accountEntry = qobject_cast<AccountEntry*>(sender());
    if (!accountEntry) {
        return;
    }
    Tp::ConnectionPtr conn(accountEntry->account()->connection());
    disconnectAllSignals(conn);

    if (accountEntry->connected()) {
        QString busName = conn->busName();
        QString objectPath = conn->objectPath();

        connectAllSignals(conn);

        QDBusInterface ussdIface(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE);
        mStates[accountEntry->accountId()] = ussdIface.property("State").toString();
        mSerials[accountEntry->accountId()] = ussdIface.property("Serial").toString();
    }
}

void USSDManager::onAccountsChanged()
{
    Q_FOREACH (AccountEntry *accountEntry, TelepathyHelper::instance()->accounts()) {
        QObject::disconnect(accountEntry, SIGNAL(connectedChanged()), this, SLOT(accountConnectedChanged()));
        QObject::connect(accountEntry, SIGNAL(connectedChanged()), this, SLOT(accountConnectedChanged()));

        // disconnect all and reconnect only the online accounts
        Tp::ConnectionPtr conn(accountEntry->account()->connection());
        disconnectAllSignals(conn);
        mStates.remove(accountEntry->accountId());
        mSerials.remove(accountEntry->accountId());

        if (accountEntry->connected()) {
            QString busName = conn->busName();
            QString objectPath = conn->objectPath();

            connectAllSignals(conn);

            QDBusInterface ussdIface(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE);
            mStates[accountEntry->accountId()] = ussdIface.property("State").toString();
            mSerials[accountEntry->accountId()] = ussdIface.property("Serial").toString();
        }
    }
    Q_EMIT stateChanged(state());
    Q_EMIT activeChanged();
    Q_EMIT activeAccountIdChanged();
}

void USSDManager::onStateChanged(const QString &)
{
    Q_FOREACH (AccountEntry *accountEntry, TelepathyHelper::instance()->accounts()) {
        Tp::ConnectionPtr conn(accountEntry->account()->connection());
        if (accountEntry->connected()) {
            QString busName = conn->busName();
            QString objectPath = conn->objectPath();
            QDBusInterface ussdIface(busName, objectPath, CANONICAL_TELEPHONY_USSD_IFACE);
            mStates[accountEntry->accountId()] = ussdIface.property("State").toString();
        }
    }
    Q_EMIT stateChanged(state());
}

bool USSDManager::active() const
{
    QMap<QString, QString>::const_iterator i = mStates.constBegin();
    while (i != mStates.constEnd()) {
        if (i.value() != "idle") {
            return true;
        }
        ++i;
    }
    return false;
}

QString USSDManager::activeAccountId() const
{
    QMap<QString, QString>::const_iterator i = mStates.constBegin();
    while (i != mStates.constEnd()) {
        if (i.value() != "idle") {
            return i.key();
        }
        ++i;
    }
    return QString::null;
}

QString USSDManager::state() const
{
    QMap<QString, QString>::const_iterator i = mStates.constBegin();
    while (i != mStates.constEnd()) {
        if (i.value() != "idle") {
            return i.value();
        }
        ++i;
    }
    return "idle";
}

QString USSDManager::serial(const QString &accountId) const
{
    return mSerials[accountId];
}
