/*
 * Copyright (C) 2013 Canonical, Ltd.
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
#include "telepathyhelper.h"

namespace C {
#include <libintl.h>
}

DisplayNameSettings::DisplayNameSettings(QObject *parent) :
    QObject(parent)
{
    // FIXME: this should go away once the system settings for account names lands.
    mAccountNames["ofono/ofono/account0"] = C::gettext("SIM 1");
    mAccountNames["ofono/ofono/account1"] = C::gettext("SIM 2");

    connect(TelepathyHelper::instance(),
            SIGNAL(accountsChanged()),
            SLOT(onAccountsChanged()));

    onAccountsChanged();
}

DisplayNameSettings *DisplayNameSettings::instance()
{
    static DisplayNameSettings *self = new DisplayNameSettings();
    return self;
}

void DisplayNameSettings::onAccountsChanged()
{
    // FIXME: retrieve the names from system settings
    Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->accounts()) {
        if (mAccountNames.contains(account->accountId()) && account->displayName() != mAccountNames[account->accountId()]) {
            account->setDisplayName(mAccountNames[account->accountId()]);
        }
    }
}
