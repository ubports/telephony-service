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

#ifndef MULTIMEDIAACCOUNTENTRY_H
#define MULTIMEDIAACCOUNTENTRY_H

#include "accountentry.h"

class MultimediaAccountEntry : public AccountEntry
{
    Q_OBJECT
    friend class AccountEntryFactory;

public:
    // reimplemented from AccountEntry
    virtual AccountEntry::AccountType type() const;
    virtual bool connected() const;
    virtual bool compareIds(const QString &first, const QString &second) const;
    virtual QStringList addressableVCardFields();

private Q_SLOTS:
    // reimplemented from AccountEntry
    void onConnectionChanged(Tp::ConnectionPtr connection);

protected:
    explicit MultimediaAccountEntry(const Tp::AccountPtr &account, QObject *parent = 0);
};

#endif // MULTIMEDIAACCOUNTENTRY_H
