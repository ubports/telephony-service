/*
 * Copyright (C) 2012-2017 Canonical, Ltd.
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
#include "accountentry.h"
#include "protocol.h"

#define CANONICAL_TELEPHONY_VOICEMAIL_IFACE "com.canonical.Telephony.Voicemail"
#define CANONICAL_TELEPHONY_USSD_IFACE "com.canonical.Telephony.USSD"
#define CANONICAL_TELEPHONY_EMERGENCYMODE_IFACE "com.canonical.Telephony.EmergencyMode"

template<> bool qMapLessThanKey<QStringList>(const QStringList &key1, const QStringList &key2);

class AccountList;

class TelepathyHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready NOTIFY setupReady)
    Q_PROPERTY(QStringList accountIds READ accountIds NOTIFY accountIdsChanged)
    Q_PROPERTY(AccountList *accounts READ qmlAccounts CONSTANT)
    Q_PROPERTY(AccountList *voiceAccounts READ qmlVoiceAccounts CONSTANT)
    Q_PROPERTY(AccountList *textAccounts READ qmlTextAccounts CONSTANT)
    Q_PROPERTY(AccountList *phoneAccounts READ qmlPhoneAccounts CONSTANT)
    Q_PROPERTY(AccountEntry *defaultMessagingAccount READ defaultMessagingAccount NOTIFY defaultMessagingAccountChanged)
    Q_PROPERTY(AccountEntry *defaultCallAccount READ defaultCallAccount NOTIFY defaultCallAccountChanged)
    Q_PROPERTY(bool flightMode READ flightMode WRITE setFlightMode NOTIFY flightModeChanged)
    Q_PROPERTY(bool mmsEnabled READ mmsEnabled WRITE setMmsEnabled NOTIFY mmsEnabledChanged)
    Q_PROPERTY(bool emergencyCallsAvailable READ emergencyCallsAvailable NOTIFY emergencyCallsAvailableChanged)
    Q_PROPERTY(bool dialpadSoundsEnabled READ dialpadSoundsEnabled WRITE setDialpadSoundsEnabled NOTIFY dialpadSoundsEnabledChanged)
    Q_PROPERTY(QVariantMap simNames READ simNames NOTIFY simNamesChanged)
    Q_ENUMS(AccountType)
    Q_ENUMS(ChatType)
public:
    enum AccountType {
        Messaging = Protocol::TextChats,
        Voice = Protocol::VoiceCalls,
    };
    Q_DECLARE_FLAGS(AccountTypes, AccountType)

    enum ChatType {
        ChatTypeNone = Tp::HandleTypeNone,
        ChatTypeContact = Tp::HandleTypeContact,
        ChatTypeRoom = Tp::HandleTypeRoom
    };

    ~TelepathyHelper();

    static TelepathyHelper *instance();

    /********************************* Account stuff *********************************/
    QList<AccountEntry*> accounts() const;
    QList<AccountEntry*> phoneAccounts() const;
    QList<AccountEntry*> activeAccounts() const;
    AccountList *qmlAccounts() const;
    AccountList *qmlVoiceAccounts() const;
    AccountList *qmlTextAccounts() const;
    AccountList *qmlPhoneAccounts() const;
    ChannelObserver *channelObserver() const;
    QDBusInterface *handlerInterface() const;
    QDBusInterface *approverInterface() const;
    AccountEntry *defaultMessagingAccount() const;
    AccountEntry *defaultCallAccount() const;
    AccountEntry *accountForConnection(const Tp::ConnectionPtr &connection) const;
    Q_INVOKABLE AccountEntry *accountForId(const QString &accountId) const;
    Q_INVOKABLE void setDefaultAccount(AccountType type, AccountEntry* account);
    Q_INVOKABLE QList<AccountEntry*> accountsForType(int type);
    bool multiplePhoneAccounts() const;

    /** @brief Check if this account should be replaced by any overloaded protocol. */
    QList<AccountEntry*> checkAccountOverload(AccountEntry *originalAccount);

    /** @brief Check if this account has a fallback to be used when the original account is not suitable. */
    QList<AccountEntry*> checkAccountFallback(AccountEntry *originalAccount);

    /* Convenience functions to be used by qml */
    Q_INVOKABLE QList<QObject*> accountOverload(AccountEntry *originalAccount);
    Q_INVOKABLE QList<QObject*> accountFallback(AccountEntry *originalAccount);

    bool mmsEnabled();
    QVariantMap simNames() const;
    void setMmsEnabled(bool value);
    bool flightMode();
    void setFlightMode(bool value);
    bool ready() const;
    QStringList accountIds();
    bool emergencyCallsAvailable() const;
    bool dialpadSoundsEnabled() const;
    void setDialpadSoundsEnabled(bool enabled);
    Q_INVOKABLE void unlockSimCards() const;

    bool registerClient(Tp::AbstractClient *client, QString name);
    bool unregisterClient(Tp::AbstractClient *client);

    // pre-populated channel class specs for conferences
    static Tp::ChannelClassSpec audioConferenceSpec();

Q_SIGNALS:
    void channelObserverCreated(ChannelObserver *observer);
    void channelObserverUnregistered();
    void accountIdsChanged();
    void accountsChanged();
    void accountAdded(AccountEntry *account);
    void phoneAccountsChanged();
    void activeAccountsChanged();
    void setupReady();
    void defaultMessagingAccountChanged();
    void defaultCallAccountChanged();
    void flightModeChanged();
    void emergencyCallsAvailableChanged();
    void mmsEnabledChanged();
    void simNamesChanged();
    void dialpadSoundsEnabledChanged();

public Q_SLOTS:
    Q_INVOKABLE void registerChannelObserver(const QString &observerName = QString::null);
    Q_INVOKABLE void unregisterChannelObserver();

protected:
    explicit TelepathyHelper(QObject *parent = 0);
    void setupAccountEntry(AccountEntry *entry);

private Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onAccountReady();
    void onNewAccount(const Tp::AccountPtr &account);
    void onAccountRemoved();
    void onPhoneSettingsChanged(const QString&);

private:
    Tp::AccountManagerPtr mAccountManager;
    Tp::Features mAccountManagerFeatures;
    Tp::Features mAccountFeatures;
    Tp::Features mContactFeatures;
    Tp::Features mConnectionFeatures;
    Tp::ClientRegistrarPtr mClientRegistrar;
    QList<AccountEntry*> mAccounts;
    int mPendingAccountReady;
    AccountList *mQmlAccounts;
    AccountList *mQmlVoiceAccounts;
    AccountList *mQmlTextAccounts;
    AccountList *mQmlPhoneAccounts;
    AccountEntry *mDefaultCallAccount;
    AccountEntry *mDefaultMessagingAccount;
    ChannelObserver *mChannelObserver;
    bool mReady;
    Tp::AbstractClientPtr mChannelObserverPtr;
    bool mMmsEnabled;
    QVariantMap mSimNames;
    mutable QDBusInterface *mHandlerInterface;
    mutable QDBusInterface *mApproverInterface;
    QDBusInterface mFlightModeInterface;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TelepathyHelper::AccountTypes)

#endif // TELEPATHYHELPER_H
