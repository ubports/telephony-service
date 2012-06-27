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

#include "contactphonenumber.h"
#include <QDebug>

ContactPhoneNumber::ContactPhoneNumber(const QContactDetail &detail, QObject *parent) :
    ContactDetail(detail, parent)
{
    connect(this,
            SIGNAL(detailChanged()),
            SIGNAL(changed()));
}

int ContactPhoneNumber::type() const
{
    return PhoneNumber;
}

QString ContactPhoneNumber::number() const
{
    return mDetail.value(QContactPhoneNumber::FieldNumber);
}

void ContactPhoneNumber::setNumber(const QString &value)
{
    if (value != number()) {
        mDetail.setValue(QContactPhoneNumber::FieldNumber, value);
        emit changed();
    }
}

QVariant ContactPhoneNumber::subTypes() const
{
    return mDetail.value<QStringList>(QContactPhoneNumber::FieldSubTypes);
}

void ContactPhoneNumber::setSubTypes(const QVariant &value)
{
    qDebug() << "BLABLA setting subtypes:" << value;
    //FIXME: we are just setting this one all the time, maybe we should check if the list really changed
    mDetail.setValue(QContactPhoneNumber::FieldSubTypes, value);
    emit changed();
}
