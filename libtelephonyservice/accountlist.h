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

#ifndef ACCOUNTLIST_H
#define ACCOUNTLIST_H

#include <QObject>
#include <QQmlListProperty>
#include "telepathyhelper.h"

class AccountEntry;

class AccountList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<AccountEntry> all READ qmlAllAccounts NOTIFY allAccountsChanged)
    Q_PROPERTY(QQmlListProperty<AccountEntry> active READ qmlActiveAccounts NOTIFY activeAccountsChanged)
    Q_PROPERTY(QQmlListProperty<AccountEntry> displayed READ qmlDisplayedAccounts NOTIFY displayedAccountsChanged)
public:
    AccountList(Protocol::Features features = Protocol::AllFeatures, const QString &protocol = QString::null, QObject *parent = 0);
    QQmlListProperty<AccountEntry> qmlAllAccounts();
    QQmlListProperty<AccountEntry> qmlActiveAccounts();
    QQmlListProperty<AccountEntry> qmlDisplayedAccounts();

    static int allAccountsCount(QQmlListProperty<AccountEntry> *p);
    static AccountEntry *allAccountsAt(QQmlListProperty<AccountEntry> *p, int index);
    static int activeAccountsCount(QQmlListProperty<AccountEntry> *p);
    static AccountEntry *activeAccountsAt(QQmlListProperty<AccountEntry> *p, int index);
    static int displayedAccountsCount(QQmlListProperty<AccountEntry> *p);
    static AccountEntry *displayedAccountsAt(QQmlListProperty<AccountEntry> *p, int index);

    QList<AccountEntry*> accounts();
    QList<AccountEntry*> activeAccounts();
    QList<AccountEntry*> displayedAccounts();

Q_SIGNALS:
    void allAccountsChanged();
    void activeAccountsChanged();
    void displayedAccountsChanged();
    void accountChanged(AccountEntry *entry, bool active);

protected Q_SLOTS:
    void init();
    void filterAccounts();
    void onActiveAccountsChanged();

private:
    Protocol::Features mFeatures;
    QString mProtocol;
    QList<AccountEntry*> mAccounts;
};



#endif // ACCOUNTLIST_H
