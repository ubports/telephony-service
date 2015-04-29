/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#ifndef TELEPATHYHELPER_H
#define TELEPATHYHELPER_H

#include <QObject>
#include <QQmlListProperty>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Contact>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ConnectionManager>
#include <TelepathyQt/Types>
#include "channelobserver.h"

#define CANONICAL_TELEPHONY_VOICEMAIL_IFACE "com.canonical.Telephony.Voicemail"
#define CANONICAL_TELEPHONY_AUDIOOUTPUTS_IFACE "com.canonical.Telephony.AudioOutputs"
#define CANONICAL_TELEPHONY_USSD_IFACE "com.canonical.Telephony.USSD"
#define CANONICAL_TELEPHONY_EMERGENCYMODE_IFACE "com.canonical.Telephony.EmergencyMode"

template<> bool qMapLessThanKey<QStringList>(const QStringList &key1, const QStringList &key2);

class AccountEntry;
class QGSettings;

class TelepathyHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QStringList accountIds READ accountIds NOTIFY accountIdsChanged)
    Q_PROPERTY(QQmlListProperty<AccountEntry> accounts READ qmlAccounts NOTIFY accountsChanged)
    Q_PROPERTY(QQmlListProperty<AccountEntry> activeAccounts READ qmlActiveAccounts NOTIFY activeAccountsChanged)
    Q_PROPERTY(AccountEntry *defaultMessagingAccount READ defaultMessagingAccount NOTIFY defaultMessagingAccountChanged)
    Q_PROPERTY(AccountEntry *defaultCallAccount READ defaultCallAccount NOTIFY defaultCallAccountChanged)
    Q_PROPERTY(bool flightMode READ flightMode WRITE setFlightMode NOTIFY flightModeChanged)
    Q_PROPERTY(bool mmsGroupChat READ mmsGroupChat WRITE setMmsGroupChat NOTIFY mmsGroupChatChanged)
    Q_PROPERTY(bool emergencyCallsAvailable READ emergencyCallsAvailable NOTIFY emergencyCallsAvailableChanged)
    Q_ENUMS(AccountType)
public:
    enum AccountType {
        Call,
        Messaging
    };

    ~TelepathyHelper();

    static TelepathyHelper *instance();
    QList<AccountEntry*> accounts() const;
    QList<AccountEntry*> activeAccounts() const;
    QQmlListProperty<AccountEntry> qmlAccounts();
    QQmlListProperty<AccountEntry> qmlActiveAccounts();
    ChannelObserver *channelObserver() const;
    QDBusInterface *handlerInterface() const;
    QDBusInterface *approverInterface() const;
    AccountEntry *defaultMessagingAccount() const;
    AccountEntry *defaultCallAccount() const;

    bool mmsGroupChat();
    void setMmsGroupChat(bool value);
    bool flightMode();
    void setFlightMode(bool value);
    bool connected() const;
    QStringList accountIds();
    AccountEntry *accountForConnection(const Tp::ConnectionPtr &connection) const;
    Q_INVOKABLE AccountEntry *accountForId(const QString &accountId) const;
    Q_INVOKABLE void setDefaultAccount(AccountType type, AccountEntry* account);
    bool emergencyCallsAvailable() const;
    Q_INVOKABLE void unlockSimCards() const;

    bool registerClient(Tp::AbstractClient *client, QString name);
    bool unregisterClient(Tp::AbstractClient *client);

    // pre-populated channel class specs for conferences
    static Tp::ChannelClassSpec audioConferenceSpec();

    // QQmlListProperty helpers
    static int accountsCount(QQmlListProperty<AccountEntry> *p);
    static AccountEntry *accountAt(QQmlListProperty<AccountEntry> *p, int index);
    static int activeAccountsCount(QQmlListProperty<AccountEntry> *p);
    static AccountEntry *activeAccountAt(QQmlListProperty<AccountEntry> *p, int index);

Q_SIGNALS:
    void channelObserverCreated(ChannelObserver *observer);
    void channelObserverUnregistered();
    void accountReady();
    void connectedChanged();
    void accountIdsChanged();
    void accountsChanged();
    void accountAdded(AccountEntry *account);
    void activeAccountsChanged();
    void setupReady();
    void defaultMessagingAccountChanged();
    void defaultCallAccountChanged();
    void flightModeChanged();
    void emergencyCallsAvailableChanged();
    void mmsGroupChatChanged();

public Q_SLOTS:
    Q_INVOKABLE void registerChannelObserver(const QString &observerName = QString::null);
    Q_INVOKABLE void unregisterChannelObserver();
    QStringList supportedProtocols() const;

protected:
    void setupAccountEntry(AccountEntry *entry);

private Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onAccountReady();
    void onNewAccount(const Tp::AccountPtr &account);
    void onAccountRemoved();
    void updateConnectedStatus();
    void onSettingsChanged(const QString&);

private:
    explicit TelepathyHelper(QObject *parent = 0);
    Tp::AccountManagerPtr mAccountManager;
    Tp::Features mAccountManagerFeatures;
    Tp::Features mAccountFeatures;
    Tp::Features mContactFeatures;
    Tp::Features mConnectionFeatures;
    Tp::ClientRegistrarPtr mClientRegistrar;
    QList<AccountEntry*> mAccounts;
    AccountEntry *mDefaultCallAccount;
    AccountEntry *mDefaultMessagingAccount;
    ChannelObserver *mChannelObserver;
    bool mFirstTime;
    bool mConnected;
    bool mMmsGroupChat;
    mutable QDBusInterface *mHandlerInterface;
    QGSettings *mPhoneSettings;
    mutable QDBusInterface *mApproverInterface;
    QDBusInterface mFlightModeInterface;
};

#endif // TELEPATHYHELPER_H
