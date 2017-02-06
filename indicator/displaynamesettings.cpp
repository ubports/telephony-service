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

#include "displaynamesettings.h"
#include "accountentry.h"
#include "greetercontacts.h"
#include "telepathyhelper.h"

namespace C {
#include <libintl.h>
}

#define DUAL_SIM_NAMES_KEY "SimNames"

// do not remove the following line.
// it is used by the the ofono-setup script when creating new accounts
#define SIM_DEFAULT_NAME C::gettext("SIM %1")

DisplayNameSettings::DisplayNameSettings(QObject *parent) :
    QObject(parent)
{
    connect(TelepathyHelper::instance(),
            SIGNAL(accountsChanged()),
            SLOT(onAccountsChanged()));

    connect(GreeterContacts::instance(),
            SIGNAL(phoneSettingsChanged(QString)),
            SLOT(onSettingsChanged(QString)));

    QVariantMap newSimNames;
    QVariantMap values = GreeterContacts::instance()->simNames();
    // if there are no sim names at this point, we have to fallback to the default names.
    // it means the migration script did not find anything on gsettings to be migrated,
    // so this is likely to be a first fresh boot
    if (values.isEmpty()) {
        int index = 1;
        Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->phoneAccounts()) {
            QString modemObjName = account->account()->parameters().value("modem-objpath").toString();
            QString newSimName = QString(SIM_DEFAULT_NAME).arg(index++);
            account->setDisplayName(newSimName);
            newSimNames[modemObjName] = newSimName;
        }
        if (!newSimNames.isEmpty()) {
            GreeterContacts::instance()->setSimNames(newSimNames);
        }
    }

    // force update during startup
    onSettingsChanged(DUAL_SIM_NAMES_KEY);
}

void DisplayNameSettings::onSettingsChanged(const QString &key)
{
    if (key == DUAL_SIM_NAMES_KEY) {
        QVariantMap values = GreeterContacts::instance()->simNames();
        for(QVariantMap::const_iterator iter = values.begin(); iter != values.end(); ++iter) {
            mAccountNames[iter.key()] = iter.value().toString();
        }
        onAccountsChanged();
    }
}

DisplayNameSettings *DisplayNameSettings::instance()
{
    static DisplayNameSettings *self = new DisplayNameSettings();
    return self;
}

void DisplayNameSettings::onAccountsChanged()
{
    Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->phoneAccounts()) {
        QString modemObjName = account->account()->parameters().value("modem-objpath").toString();
        if (mAccountNames.contains(modemObjName) && account->displayName() != mAccountNames[modemObjName]) {
            account->setDisplayName(mAccountNames[modemObjName]);
        }
    }
}
