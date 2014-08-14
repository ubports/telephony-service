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
#include <TelepathyQt/Connection>

class TelepathyHelper;

class USSDManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active 
               READ active
               NOTIFY activeChanged)
    Q_PROPERTY(QString activeAccountId
               READ activeAccountId
               NOTIFY activeAccountIdChanged)
    Q_PROPERTY(QString state 
               READ state
               NOTIFY stateChanged)
public:
    static USSDManager *instance();
    Q_INVOKABLE void initiate(const QString &command, const QString &accountId = QString::null);
    Q_INVOKABLE void respond(const QString &reply, const QString &accountId = QString::null);
    Q_INVOKABLE void cancel(const QString &accountId = QString::null);
    Q_INVOKABLE QString serial(const QString& accountId) const;

    bool active() const;
    QString activeAccountId() const;
    QString state() const;

public Q_SLOTS:
    void onAccountsChanged();
    void onStateChanged(const QString &state);
    void accountConnectedChanged();

Q_SIGNALS:
    void activeChanged();
    void activeAccountIdChanged();
    void stateChanged(const QString &state);

    void notificationReceived(const QString &message);
    void requestReceived(const QString &message);

    void initiateUSSDComplete(const QString &ussdResp);
    void respondComplete(bool success, const QString &ussdResp);
    void barringComplete(const QString &ssOp, const QString &cbService, const QVariantMap &cbMap);
    void forwardingComplete(const QString &ssOp, const QString &cfService, const QVariantMap &cfMap);
    void waitingComplete(const QString &ssOp, const QVariantMap &cwMap);
    void callingLinePresentationComplete(const QString &ssOp, const QString &status);
    void connectedLinePresentationComplete(const QString &ssOp, const QString &status);
    void callingLineRestrictionComplete(const QString &ssOp, const QString &status);
    void connectedLineRestrictionComplete(const QString &ssOp, const QString &status);
    void initiateFailed();

private:
    explicit USSDManager(QObject *parent = 0);

    Tp::ConnectionPtr connectionForAccountId(const QString &accountId = QString::null);

    void disconnectAllSignals(const Tp::ConnectionPtr& conn);
    void connectAllSignals(const Tp::ConnectionPtr& conn);

    QMap<QString, QString> mStates;
    QMap<QString, QString> mSerials;
};

#endif // USSDMANAGER_H
