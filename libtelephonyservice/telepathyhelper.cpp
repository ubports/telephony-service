/*
 * Copyright (C) 2012-2015 Canonical, Ltd.
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
#include "ofonoaccountentry.h"
#include "accountentryfactory.h"
#include "chatmanager.h"
#include "callmanager.h"
#include "config.h"
#include "greetercontacts.h"
#include "protocolmanager.h"

#include "qgsettings.h"

#include <TelepathyQt/AccountSet>
#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingAccount>

template<> bool qMapLessThanKey<QStringList>(const QStringList &key1, const QStringList &key2) 
{ 
    return key1.size() > key2.size();  // sort by operator> !
}

TelepathyHelper::TelepathyHelper(QObject *parent)
    : QObject(parent),
      mPendingAccountReady(0),
      mDefaultCallAccount(NULL),
      mDefaultMessagingAccount(NULL),
      mChannelObserver(0),
      mReady(false),
      mChannelObserverPtr(NULL),
      mHandlerInterface(0),
      mPhoneSettings(new QGSettings("com.ubuntu.phone")),
      mApproverInterface(0),
      mFlightModeInterface("org.freedesktop.URfkill",
                           "/org/freedesktop/URfkill",
                           "org.freedesktop.URfkill",
                           QDBusConnection::systemBus())
{
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
    connect(mPhoneSettings, SIGNAL(changed(QString)), this, SLOT(onSettingsChanged(QString)));
    connect(&mFlightModeInterface, SIGNAL(FlightModeChanged(bool)), this, SIGNAL(flightModeChanged()));

    mMmsGroupChat = mPhoneSettings->get("mmsGroupChatEnabled").value<bool>(); 
}

TelepathyHelper::~TelepathyHelper()
{
    mPhoneSettings->deleteLater();
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

void TelepathyHelper::setMmsGroupChat(bool enable)
{
    mPhoneSettings->set("mmsGroupChatEnabled", enable);
}

bool TelepathyHelper::mmsGroupChat()
{
    return mMmsGroupChat;
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
        if (account->active() && account->type() != AccountEntry::MultimediaAccount) {
            activeAccountList << account;
        }
    }
    return activeAccountList;
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

QQmlListProperty<AccountEntry> TelepathyHelper::qmlPhoneAccounts()
{
    return QQmlListProperty<AccountEntry>(this, 0, phoneAccountsCount, phoneAccountAt);
}

QQmlListProperty<AccountEntry> TelepathyHelper::qmlAccounts()
{
    return QQmlListProperty<AccountEntry>(this, 0, accountsCount, accountAt);
}

QQmlListProperty<AccountEntry> TelepathyHelper::qmlActiveAccounts()
{
    return QQmlListProperty<AccountEntry>(this, 0, activeAccountsCount, activeAccountAt);
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

int TelepathyHelper::phoneAccountsCount(QQmlListProperty<AccountEntry> *p)
{
    Q_UNUSED(p)
    return TelepathyHelper::instance()->phoneAccounts().count();
}

int TelepathyHelper::accountsCount(QQmlListProperty<AccountEntry> *p)
{
    Q_UNUSED(p)
    return TelepathyHelper::instance()->accounts().count();
}

AccountEntry *TelepathyHelper::phoneAccountAt(QQmlListProperty<AccountEntry> *p, int index)
{
    Q_UNUSED(p)
    return TelepathyHelper::instance()->phoneAccounts()[index];
}

AccountEntry *TelepathyHelper::accountAt(QQmlListProperty<AccountEntry> *p, int index)
{
    Q_UNUSED(p)
    return TelepathyHelper::instance()->accounts()[index];
}

int TelepathyHelper::activeAccountsCount(QQmlListProperty<AccountEntry> *p)
{
    return TelepathyHelper::instance()->activeAccounts().count();
}

AccountEntry *TelepathyHelper::activeAccountAt(QQmlListProperty<AccountEntry> *p, int index)
{
    return TelepathyHelper::instance()->activeAccounts()[index];
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
    onSettingsChanged("defaultSimForMessages");
    onSettingsChanged("defaultSimForCalls");
}

void TelepathyHelper::onNewAccount(const Tp::AccountPtr &account)
{
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
    onSettingsChanged("defaultSimForMessages");
    onSettingsChanged("defaultSimForCalls");
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
    onSettingsChanged("defaultSimForMessages");
    onSettingsChanged("defaultSimForCalls");
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

void TelepathyHelper::setDefaultAccount(AccountType type, AccountEntry* account)
{
    if (!account) {
        return;
    }

    QString modemObjName = account->account()->parameters().value("modem-objpath").toString();
    if (!modemObjName.isEmpty()) {
        if (type == Call) {
            mPhoneSettings->set("defaultSimForCalls", modemObjName);
        } else if (type == Messaging) {
            mPhoneSettings->set("defaultSimForMessages", modemObjName);
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

void TelepathyHelper::onSettingsChanged(const QString &key)
{
    if (key == "defaultSimForMessages") {
        QString defaultSim = mPhoneSettings->get("defaultSimForMessages").value<QString>(); 
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
    } else if (key == "defaultSimForCalls") {
        QString defaultSim = mPhoneSettings->get("defaultSimForCalls").value<QString>(); 
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
    } else if (key == "mmsGroupChatEnabled") {
        mMmsGroupChat = mPhoneSettings->get("mmsGroupChatEnabled").value<bool>(); 
        Q_EMIT mmsGroupChatChanged();
    }
}

void TelepathyHelper::unlockSimCards() const
{
    QDBusInterface connectivityIface("com.ubuntu.connectivity1",
                                    "/com/ubuntu/connectivity1/Private",
                                    "com.ubuntu.connectivity1.Private");
    connectivityIface.asyncCall("UnlockAllModems");
}

