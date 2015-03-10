/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include "accountentryfactory.h"
#include "accountentry.h"
#include "ofonoaccountentry.h"

AccountEntry *AccountEntryFactory::createEntry(const Tp::AccountPtr &account, QObject *parent)
{
    QString protocol = account.isNull() ? "" : account->protocolName();

    // FIXME: check what other accounts need extra properties/methods
    if (protocol == "ofono") {
        return new OfonoAccountEntry(account, parent);
    }

    return new AccountEntry(account, parent);
}
