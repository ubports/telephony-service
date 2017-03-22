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

#include "telepathyhelper.h"
#include "accountentry.h"
#include "accountlist.h"
#include "ofonoaccountentry.h"
#include "accountentryfactory.h"
#include "chatmanager.h"
#include "callmanager.h"
#include "config.h"
#include "greetercontacts.h"
#include "protocolmanager.h"

#include <TelepathyQt/AccountSet>
#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingAccount>

template<> bool qMapLessThanKey<QStringList>(const QStringList &key1, const QStringList &key2) 
{ 
    return key1.size() > key2.size();  // sort by operator> !
}

typedef struct {
    QList<AccountEntry*> (*accountFunction)();
} QmlAccountData;

TelepathyHelper::TelepathyHelper(QObject *parent)
    : QObject(parent),
      mPendingAccountReady(0),
      mDefaultCallAccount(NULL),
      mDefaultMessagingAccount(NULL),
      mChannelObserver(0),
      mReady(false),
      mChannelObserverPtr(NULL),
      mHandlerInterface(0),
      mApproverInterface(0),
      mFlightModeInterface("org.freedesktop.URfkill",
                           "/org/freedesktop/URfkill",
                           "org.freedesktop.URfkill",
                           QDBusConnection::systemBus())
{
    mQmlAccounts = new AccountList(Protocol::AllFeatures, QString::null, this);
    mQmlVoiceAccounts = new AccountList(Protocol::VoiceCalls, QString::null, this);
    mQmlTextAccounts = new AccountList(Protocol::TextChats, QString::null, this);
    mQmlPhoneAccounts = new AccountList(Protocol::AllFeatures, "ofono", this);

    qRegisterMetaType<QList<AccountEntry*> >();
    mAccountFeatures << Tp::Account::FeatureCore
                     << Tp::Account::FeatureProtocolInfo;
    mContactFeatures << Tp::Contact::FeatureAlias
                     << Tp::Contact::FeatureAvatarData
                     << Tp::Contact::FeatureAvatarToken
                     << Tp::Contact::FeatureCapabilities
                     << Tp::Contact::FeatureSimplePresence;
    mConnectionFeatures << Tp::Connection::FeatureCore
                        << Tp::Connection::FeatureSelfContact
                        << Tp::Connection::FeatureSimplePresence;

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
    channelFactory->addCommonFeatures(Tp::Channel::FeatureCore);

    // FIXME: at some point this needs to be fixed in tp-qt itself.
    channelFactory->setSubclassFor<Tp::CallChannel>(audioConferenceSpec());

    mAccountManager = Tp::AccountManager::create(
            Tp::AccountFactory::create(QDBusConnection::sessionBus(), mAccountFeatures),
            Tp::ConnectionFactory::create(QDBusConnection::sessionBus(), mConnectionFeatures),
            channelFactory,
            Tp::ContactFactory::create(mContactFeatures));

    connect(mAccountManager->becomeReady(Tp::AccountManager::FeatureCore),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));

    mClientRegistrar = Tp::ClientRegistrar::create(mAccountManager);
    connect(GreeterContacts::instance(), SIGNAL(phoneSettingsChanged(QString)), this, SLOT(onPhoneSettingsChanged(QString)));
    connect(GreeterContacts::instance(), SIGNAL(soundSettingsChanged(QString)), this, SLOT(onPhoneSettingsChanged(QString)));
    connect(&mFlightModeInterface, SIGNAL(FlightModeChanged(bool)), this, SIGNAL(flightModeChanged()));

    mMmsEnabled = GreeterContacts::instance()->mmsEnabled();
}

TelepathyHelper::~TelepathyHelper()
{
}

TelepathyHelper *TelepathyHelper::instance()
{
    static TelepathyHelper* helper = new TelepathyHelper();
    return helper;
}

QStringList TelepathyHelper::accountIds()
{
    QStringList ids;

    if (QCoreApplication::applicationName() == "telephony-service-handler" || mAccounts.size() != 0) {
        Q_FOREACH(const AccountEntry *account, mAccounts) {
            ids << account->accountId();
        }
    } else if (!GreeterContacts::instance()->isGreeterMode()) {
        // if we are in greeter mode, we should not initialize the handler to get the account IDs
        QDBusReply<QStringList> reply = handlerInterface()->call("AccountIds");
        if (reply.isValid()) {
            ids = reply.value();
        }
    }

    return ids;
}

void TelepathyHelper::setMmsEnabled(bool enable)
{
    GreeterContacts::instance()->setMmsEnabled(enable);
}

bool TelepathyHelper::mmsEnabled()
{
    return mMmsEnabled;
}

bool TelepathyHelper::flightMode()
{
    QDBusReply<bool> reply = mFlightModeInterface.call("IsFlightMode");
    if (reply.isValid()) {
        return reply;
    }
    return false;
}

void TelepathyHelper::setFlightMode(bool value)
{
    mFlightModeInterface.asyncCall("FlightMode", value);
}

QList<AccountEntry*> TelepathyHelper::accounts() const
{
    return mAccounts;
}

QList<AccountEntry*> TelepathyHelper::activeAccounts() const
{
    QList<AccountEntry*> activeAccountList;
    Q_FOREACH(AccountEntry *account, mAccounts) {
        if (account->active()) {
            activeAccountList << account;
        }
    }
    return activeAccountList;
}

AccountList *TelepathyHelper::qmlAccounts() const
{
    return mQmlAccounts;
}

AccountList *TelepathyHelper::qmlVoiceAccounts() const
{
    return mQmlVoiceAccounts;
}

AccountList *TelepathyHelper::qmlTextAccounts() const
{
    return mQmlTextAccounts;
}

AccountList *TelepathyHelper::qmlPhoneAccounts() const
{
    return mQmlPhoneAccounts;
}

bool TelepathyHelper::multiplePhoneAccounts() const
{
    int count = 0;
    Q_FOREACH(AccountEntry *account, phoneAccounts()) {
        if (account->active()) {
            count++;
        }
    }
    return (count > 1);
}

/**
 * If the @param originalAccount is listed as being the fallback of any other protocol,
 * this function will return the account that should be used instead.
 * @param originalAccount The original account that might be replaced
 * @return A list containing the replacement accounts or an empty list if none are suitable.
 */
QList<AccountEntry*> TelepathyHelper::checkAccountOverload(AccountEntry *originalAccount)
{
    QList<AccountEntry*> accounts;

    if (!originalAccount) {
        return accounts;
    }

    QString protocol = originalAccount->protocolInfo()->name();
    for (auto account : mAccounts) {
        // FIXME: check for matching properties if needed
        if (account->protocolInfo()->fallbackProtocol() == protocol) {
            accounts << account;
        }
    }

    return accounts;
}

/**
 * If the @param originalAccount is listed as having a fallback protocol, when the @param originalAccount
 * cannot be used by any reason (not connected, or not having a particular feature enabled), the fallback
 * account should be used. This function will try to find a suitable fallback account.
 * @return A list containing the fallback accounts or an empty list if none are suitable.
 */
QList<AccountEntry*> TelepathyHelper::checkAccountFallback(AccountEntry *originalAccount)
{
    QList<AccountEntry*> accounts;

    if (!originalAccount) {
        return accounts;
    }

    QString fallbackProtocol = originalAccount->protocolInfo()->fallbackProtocol();
    // FIXME: check for the match rules too

    // if the account doesn't have a fallback protocol specified, just return the empty list
    if (fallbackProtocol.isEmpty()) {
        return accounts;
    }

    for (auto account : mAccounts) {
        if (account->protocolInfo()->name() == fallbackProtocol) {
            accounts << account;
        }
    }

    return accounts;
}

QList<QObject*> TelepathyHelper::accountOverload(AccountEntry *originalAccount)
{
    QList<QObject*> accounts;
    for (auto account : checkAccountOverload(originalAccount)) {
        accounts << account;
    }
    return accounts;
}

QList<QObject*> TelepathyHelper::accountFallback(AccountEntry *originalAccount)
{
    QList<QObject*> accounts;
    for (auto account : checkAccountFallback(originalAccount)) {
        accounts << account;
    }
    return accounts;
}

QList<AccountEntry*> TelepathyHelper::phoneAccounts() const
{
    QList<AccountEntry*> accountList;
    Q_FOREACH(AccountEntry *account, mAccounts) {
        if (account->type() == AccountEntry::PhoneAccount) {
            accountList << account;
        }
    }
    return accountList;
}

ChannelObserver *TelepathyHelper::channelObserver() const
{
    return mChannelObserver;
}

QDBusInterface *TelepathyHelper::handlerInterface() const
{
    // delay the loading of the handler interface, as it seems this is triggering
    // the dbus activation of the handler process
    if (!mHandlerInterface) {
        mHandlerInterface = new QDBusInterface("com.canonical.TelephonyServiceHandler",
                                               "/com/canonical/TelephonyServiceHandler",
                                               "com.canonical.TelephonyServiceHandler",
                                               QDBusConnection::sessionBus(),
                                               const_cast<TelepathyHelper*>(this));
    }
    return mHandlerInterface;
}

QDBusInterface *TelepathyHelper::approverInterface() const
{
    if (!mApproverInterface) {
        mApproverInterface = new QDBusInterface("org.freedesktop.Telepathy.Client.TelephonyServiceApprover",
                                               "/com/canonical/Approver",
                                               "com.canonical.TelephonyServiceApprover",
                                               QDBusConnection::sessionBus(),
                                               const_cast<TelepathyHelper*>(this));
    }
    return mApproverInterface;
}

bool TelepathyHelper::ready() const
{
    return mReady;
}

void TelepathyHelper::registerChannelObserver(const QString &observerName)
{
    QString name = observerName;

    if (name.isEmpty()) {
        name = "TelephonyPluginObserver";
    }

    if (mChannelObserver) {
        unregisterClient(mChannelObserver);
    }

    mChannelObserver = new ChannelObserver(this);
    mChannelObserverPtr = Tp::AbstractClientPtr(mChannelObserver);
    if (registerClient(mChannelObserver, name)) {
        // messages
        connect(mChannelObserver, SIGNAL(textChannelAvailable(Tp::TextChannelPtr)),
                ChatManager::instance(), SLOT(onTextChannelAvailable(Tp::TextChannelPtr)));

        // calls
        connect(mChannelObserver, SIGNAL(callChannelAvailable(Tp::CallChannelPtr)),
                CallManager::instance(), SLOT(onCallChannelAvailable(Tp::CallChannelPtr)));

        Q_EMIT channelObserverCreated(mChannelObserver);
    }
}

void TelepathyHelper::unregisterChannelObserver()
{
    unregisterClient(mChannelObserver);
    mChannelObserver->deleteLater();
    mChannelObserverPtr.reset();
    mChannelObserver = NULL;
    Q_EMIT channelObserverUnregistered();
}

void TelepathyHelper::setupAccountEntry(AccountEntry *entry)
{
    connect(entry,
            SIGNAL(connectedChanged()),
            SIGNAL(activeAccountsChanged()));
    connect(entry,
            SIGNAL(accountReady()),
            SLOT(onAccountReady()));
    connect(entry,
            SIGNAL(removed()),
            SLOT(onAccountRemoved()));

    OfonoAccountEntry *ofonoAccount = qobject_cast<OfonoAccountEntry*>(entry);
    if (ofonoAccount) {
        connect(ofonoAccount,
                SIGNAL(emergencyCallsAvailableChanged()),
                SIGNAL(emergencyCallsAvailableChanged()));
    }
}

bool TelepathyHelper::registerClient(Tp::AbstractClient *client, QString name)
{
    Tp::AbstractClientPtr clientPtr(client);
    bool succeeded = mClientRegistrar->registerClient(clientPtr, name);
    if (!succeeded) {
        name.append("%1");
        int count = 0;
        // limit the number of registered clients to 20, that should be a safe margin
        while (!succeeded && count < 20) {
            succeeded = mClientRegistrar->registerClient(clientPtr, name.arg(++count));
            if (succeeded) {
                name = name.arg(count);
            }
        }
    }

    if (succeeded) {
        QObject *object = dynamic_cast<QObject*>(client);
        if (object) {
            object->setProperty("clientName", TP_QT_IFACE_CLIENT + "." + name );
        }
    }

    return succeeded;
}

bool TelepathyHelper::unregisterClient(Tp::AbstractClient *client)
{
    Tp::AbstractClientPtr clientPtr(client);
    if (clientPtr) {
        return mClientRegistrar->unregisterClient(clientPtr);
    }
    return false;
}

AccountEntry *TelepathyHelper::accountForConnection(const Tp::ConnectionPtr &connection) const
{
    if (connection.isNull()) {
        return 0;
    }

    Q_FOREACH(AccountEntry *accountEntry, mAccounts) {
        if (accountEntry->account()->connection() == connection) {
            return accountEntry;
        }
    }

    return 0;
}

AccountEntry *TelepathyHelper::accountForId(const QString &accountId) const
{
    Q_FOREACH(AccountEntry *account, mAccounts) {
        if (account->accountId() == accountId) {
            return account;
        }
    }

    return 0;
}

Tp::ChannelClassSpec TelepathyHelper::audioConferenceSpec()
{
    static Tp::ChannelClassSpec spec;
    if (!spec.isValid()) {
        spec = Tp::ChannelClassSpec(TP_QT_IFACE_CHANNEL_TYPE_CALL, Tp::HandleTypeNone);
        spec.setCallInitialAudioFlag();
    }
    return spec;
}

void TelepathyHelper::onAccountRemoved()
{
    AccountEntry *account = qobject_cast<AccountEntry*>(sender());
    if (!account) {
        return;
    }
    mAccounts.removeAll(account);

    Q_EMIT accountIdsChanged();
    Q_EMIT accountsChanged();
    Q_EMIT phoneAccountsChanged();
    Q_EMIT activeAccountsChanged();
    onPhoneSettingsChanged("DefaultSimForMessages");
    onPhoneSettingsChanged("DefaultSimForCalls");
}

void TelepathyHelper::onNewAccount(const Tp::AccountPtr &account)
{
    if (!ProtocolManager::instance()->protocolByName(account->protocolName())) {
       return;
    }
    AccountEntry *accountEntry = AccountEntryFactory::createEntry(account, this);
    setupAccountEntry(accountEntry);
    mAccounts.append(accountEntry);

    QMap<QString, AccountEntry *> sortedOfonoAccounts;
    QMap<QString, AccountEntry *> sortedOtherAccounts;
    Q_FOREACH(AccountEntry *account, mAccounts) {
        QString modemObjName = account->account()->parameters().value("modem-objpath").toString();
        if (modemObjName.isEmpty()) {
            sortedOtherAccounts[account->accountId()] = account;
        } else {
            sortedOfonoAccounts[modemObjName] = account;
        }
    }
    mAccounts = QList<AccountEntry*>() << sortedOfonoAccounts.values() <<  sortedOtherAccounts.values() ;

    Q_EMIT accountIdsChanged();
    Q_EMIT accountsChanged();
    Q_EMIT phoneAccountsChanged();
    Q_EMIT activeAccountsChanged();
    onPhoneSettingsChanged("DefaultSimForMessages");
    onPhoneSettingsChanged("DefaultSimForCalls");
    Q_EMIT accountAdded(accountEntry);
}

void TelepathyHelper::onAccountManagerReady(Tp::PendingOperation *op)
{
    // if the account manager ready job returns an error, just fail silently
    if (op->isError()) {
        qCritical() << "Failed to prepare Tp::AccountManager" << op->errorName() << op->errorMessage();
        return;
    }

    // handle dynamic account adding and removing
    connect(mAccountManager.data(), SIGNAL(newAccount(const Tp::AccountPtr &)), SLOT(onNewAccount(const Tp::AccountPtr &)));

    Tp::AccountSetPtr accountSet;
    // try to find an account of the one of supported protocols
    Q_FOREACH(const QString &protocol, ProtocolManager::instance()->protocolNames()) {
        accountSet = mAccountManager->accountsByProtocol(protocol);
        Q_FOREACH(const Tp::AccountPtr &account, accountSet->accounts()) {
            onNewAccount(account);
        }
    }

    // get the number of pending accounts to be processed first
    mPendingAccountReady = mAccounts.count();

    if (mPendingAccountReady == 0) {
        mReady = true;
        Q_EMIT setupReady();
        return;
    }

    Q_EMIT accountIdsChanged();
    Q_EMIT accountsChanged();
    Q_EMIT phoneAccountsChanged();
    Q_EMIT activeAccountsChanged();
    onPhoneSettingsChanged("DefaultSimForMessages");
    onPhoneSettingsChanged("DefaultSimForCalls");
}

void TelepathyHelper::onAccountReady()
{
    if (mReady) {
        return;
    }

    mPendingAccountReady--;

    if (mPendingAccountReady == 0) {
        mReady = true;
        Q_EMIT setupReady();
    }
}

AccountEntry *TelepathyHelper::defaultMessagingAccount() const
{
    return mDefaultMessagingAccount;
}

AccountEntry *TelepathyHelper::defaultCallAccount() const
{
    return mDefaultCallAccount;
}

QVariantMap TelepathyHelper::simNames() const
{
    return mSimNames;
}

void TelepathyHelper::setDefaultAccount(AccountType type, AccountEntry* account)
{
    if (!account) {
        return;
    }

    QString modemObjName = account->account()->parameters().value("modem-objpath").toString();
    if (!modemObjName.isEmpty()) {
        if (type == Voice) {
            GreeterContacts::instance()->setDefaultSimForCalls(modemObjName);
        } else if (type == Messaging) {
            GreeterContacts::instance()->setDefaultSimForMessages(modemObjName);
        }
    }
}

bool TelepathyHelper::emergencyCallsAvailable() const
{
    // FIXME: this is really ofono specific, so maybe move somewhere else?
    Q_FOREACH(const AccountEntry *account, mAccounts) {
        const OfonoAccountEntry *ofonoAccount = qobject_cast<const OfonoAccountEntry*>(account);
        if (ofonoAccount && ofonoAccount->emergencyCallsAvailable()) {
            return true;
        }
    }
    return false;
}

bool TelepathyHelper::dialpadSoundsEnabled() const
{
    return GreeterContacts::instance()->dialpadSoundsEnabled();
}

void TelepathyHelper::setDialpadSoundsEnabled(bool enabled)
{
    GreeterContacts::instance()->setDialpadSoundsEnabled(enabled);
}

void TelepathyHelper::onPhoneSettingsChanged(const QString &key)
{
    if (key == "DefaultSimForMessages") {
        QString defaultSim = GreeterContacts::instance()->defaultSimForMessages();
        if (defaultSim == "ask") {
            mDefaultMessagingAccount = NULL;
            Q_EMIT defaultMessagingAccountChanged();
            return;
        }
        
        Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->accounts()) {
            QString modemObjName = account->account()->parameters().value("modem-objpath").toString();
            if (modemObjName == defaultSim) {
                mDefaultMessagingAccount = account;
                Q_EMIT defaultMessagingAccountChanged();
                return;
            }
        }
        mDefaultMessagingAccount = NULL;
        Q_EMIT defaultMessagingAccountChanged();
    } else if (key == "DefaultSimForCalls") {
        // if there is a VOIP account configured, use that by default
        // FIXME: revisit the topic and maybe discuss with designers what is best here?
        Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->qmlVoiceAccounts()->activeAccounts()) {
            if (account->type() != AccountEntry::PhoneAccount) {
                mDefaultCallAccount = account;
                Q_EMIT defaultCallAccountChanged();
                return;
            }
        }

        // if no VOIP account, get the default modem setting
        QString defaultSim = GreeterContacts::instance()->defaultSimForCalls();
        if (defaultSim == "ask") {
            mDefaultCallAccount = NULL;
            Q_EMIT defaultCallAccountChanged();
            return;
        }
        
        Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->accounts()) {
            QString modemObjName = account->account()->parameters().value("modem-objpath").toString();
            if (modemObjName == defaultSim) {
                mDefaultCallAccount = account;
                Q_EMIT defaultCallAccountChanged();
                return;
            }
        }
        mDefaultCallAccount = NULL;
        Q_EMIT defaultCallAccountChanged();
    } else if (key == "MmsEnabled") {
        mMmsEnabled = GreeterContacts::instance()->mmsEnabled();
        Q_EMIT mmsEnabledChanged();
    } else if (key == "SimNames") {
        mSimNames = GreeterContacts::instance()->simNames();
        Q_EMIT simNamesChanged();
    } else if (key == "DialpadSoundsEnabled") {
        Q_EMIT dialpadSoundsEnabledChanged();
    }
}

void TelepathyHelper::unlockSimCards() const
{
    QDBusInterface connectivityIface("com.ubuntu.connectivity1",
                                    "/com/ubuntu/connectivity1/Private",
                                    "com.ubuntu.connectivity1.Private");
    connectivityIface.asyncCall("UnlockAllModems");
}


QList<AccountEntry*> TelepathyHelper::accountsForType(int type)
{
    QList<AccountEntry*> accounts;
    Q_FOREACH(AccountEntry *account, mAccounts) {
        if (account->type() == (AccountEntry::AccountType)type) {
            accounts << account;
        }
    }
    return accounts;
}
