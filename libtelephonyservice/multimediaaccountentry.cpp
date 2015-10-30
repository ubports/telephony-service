/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

#include "multimediaaccountentry.h"
#include "phoneutils.h"
#include "telepathyhelper.h"

MultimediaAccountEntry::MultimediaAccountEntry(const Tp::AccountPtr &account, QObject *parent) :
    AccountEntry(account, parent)
{
}

AccountEntry::AccountType MultimediaAccountEntry::type() const
{
    return AccountEntry::MultimediaAccount;
}

bool MultimediaAccountEntry::connected() const
{
    return !mAccount.isNull() && !mAccount->connection().isNull() &&
           !mAccount->connection()->selfContact().isNull() &&
            mAccount->connection()->selfContact()->presence().type() != Tp::ConnectionPresenceTypeOffline;
}

bool MultimediaAccountEntry::compareIds(const QString &first, const QString &second) const
{
    return PhoneUtils::comparePhoneNumbers(first, second) > PhoneUtils::NO_MATCH;
}

QStringList MultimediaAccountEntry::addressableVCardFields()
{
    return mAccount->protocolInfo().addressableVCardFields();
}

void MultimediaAccountEntry::onConnectionChanged()
{
    // make sure the generic code is also run
    AccountEntry::onConnectionChanged();
}
