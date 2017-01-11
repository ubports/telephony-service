/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * Authors:
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

#include "accountlist.h"
#include "accountentry.h"
#include "protocol.h"
#include <QTimer>

AccountList::AccountList(Protocol::Features features, const QString &protocol, QObject *parent)
    : QObject(parent), mFeatures(features), mProtocol(protocol)
{
    QTimer::singleShot(0, this, &AccountList::init);
}

QQmlListProperty<AccountEntry> AccountList::qmlAllAccounts()
{
    return QQmlListProperty<AccountEntry>(this, 0, allAccountsCount, allAccountsAt);
}

QQmlListProperty<AccountEntry> AccountList::qmlActiveAccounts()
{
    return QQmlListProperty<AccountEntry>(this, 0, activeAccountsCount, activeAccountsAt);
}

QQmlListProperty<AccountEntry> AccountList::qmlDisplayedAccounts()
{
    return QQmlListProperty<AccountEntry>(this, 0, displayedAccountsCount, displayedAccountsAt);
}

int AccountList::allAccountsCount(QQmlListProperty<AccountEntry> *p)
{
    AccountList *accountList = qobject_cast<AccountList*>(p->object);
    return accountList->mAccounts.count();
}

AccountEntry *AccountList::allAccountsAt(QQmlListProperty<AccountEntry> *p, int index)
{
    AccountList *accountList = qobject_cast<AccountList*>(p->object);
    return accountList->mAccounts[index];
}

int AccountList::activeAccountsCount(QQmlListProperty<AccountEntry> *p)
{
    AccountList *accountList = qobject_cast<AccountList*>(p->object);
    return accountList->activeAccounts().count();
}

AccountEntry *AccountList::activeAccountsAt(QQmlListProperty<AccountEntry> *p, int index)
{
    AccountList *accountList = qobject_cast<AccountList*>(p->object);
    return accountList->activeAccounts()[index];
}

int AccountList::displayedAccountsCount(QQmlListProperty<AccountEntry> *p)
{
    AccountList *accountList = qobject_cast<AccountList*>(p->object);
    return accountList->displayedAccounts().count();
}

AccountEntry *AccountList::displayedAccountsAt(QQmlListProperty<AccountEntry> *p, int index)
{
    AccountList *accountList = qobject_cast<AccountList*>(p->object);
    return accountList->displayedAccounts()[index];
}

QList<AccountEntry*> AccountList::accounts()
{
    return mAccounts;
}

QList<AccountEntry*> AccountList::activeAccounts()
{
    QList<AccountEntry*> accounts;
    for (auto account : mAccounts) {
        if (account->active()) {
            accounts << account;
        }
    }
    return accounts;
}

QList<AccountEntry*> AccountList::displayedAccounts()
{
    QList<AccountEntry*> accounts;
    for (auto account : mAccounts) {
        if (account->active() && account->protocolInfo()->showOnSelector()) {
            accounts << account;
        }
    }
    return accounts;
}

void AccountList::init()
{
    filterAccounts();
    connect(TelepathyHelper::instance(), &TelepathyHelper::accountsChanged,
            this, &AccountList::filterAccounts);
}

void AccountList::filterAccounts()
{
    // FIXME: we need to watch for active changed on accounts
    for (auto account : mAccounts) {
        account->disconnect(this);
    }

    mAccounts.clear();
    for (auto account : TelepathyHelper::instance()->accounts()) {
        // if the account doesn't have any of the required features, skip it
        if (!account->protocolInfo()->features() & mFeatures) {
            continue;
        }

        // if we are filtering by protocol, only add the ones matching the protocol name
        if (!mProtocol.isNull() && account->protocolInfo()->name() != mProtocol) {
            continue;
        }

        connect(account, &AccountEntry::activeChanged,
                this, &AccountList::activeAccountsChanged);
        mAccounts << account;
    }

    Q_EMIT allAccountsChanged();
    Q_EMIT displayedAccountsChanged();
    Q_EMIT activeAccountsChanged();
}
