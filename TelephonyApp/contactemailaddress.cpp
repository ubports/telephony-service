/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "contactemailaddress.h"
#include <QContactEmailAddress>

ContactEmailAddress::ContactEmailAddress(const QContactDetail &detail, QObject *parent) :
    ContactDetail(detail, parent)
{
    connect(this,
            SIGNAL(detailChanged()),
            SIGNAL(changed()));
}

int ContactEmailAddress::type() const
{
    return EmailAddress;
}

QString ContactEmailAddress::emailAddress() const
{
    return mDetail.value(QContactEmailAddress::FieldEmailAddress);
}

void ContactEmailAddress::setEmailAddress(const QString &value)
{
    if (value != emailAddress()) {
        mDetail.setValue(QContactEmailAddress::FieldEmailAddress, value);
        emit changed();
    }
}
