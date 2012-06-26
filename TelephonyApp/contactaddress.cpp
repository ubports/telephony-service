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

#include "contactaddress.h"
#include <QContactAddress>

ContactAddress::ContactAddress(const QContactDetail &detail, QObject *parent) :
    ContactDetail(detail, parent)
{
    connect(this,
            SIGNAL(detailChanged()),
            SIGNAL(changed()));
}

int ContactAddress::type() const
{
    return Address;
}

QString ContactAddress::country() const
{
    return mDetail.value(QContactAddress::FieldCountry);
}

void ContactAddress::setCountry(const QString &value)
{
    if (value != country()) {
        mDetail.setValue(QContactAddress::FieldCountry, value);
        emit changed();
    }
}

QString ContactAddress::locality() const
{
    return mDetail.value(QContactAddress::FieldLocality);
}

void ContactAddress::setLocality(const QString &value)
{
    if (value != locality()) {
        mDetail.setValue(QContactAddress::FieldLocality, value);
        emit changed();
    }
}

QString ContactAddress::postOfficeBox() const
{
    return mDetail.value(QContactAddress::FieldPostOfficeBox);
}

void ContactAddress::setPostOfficeBox(const QString &value)
{
    if (value != postOfficeBox()) {
        mDetail.setValue(QContactAddress::FieldPostOfficeBox, value);
        emit changed();
    }
}

QString ContactAddress::postcode() const
{
    return mDetail.value(QContactAddress::FieldPostcode);
}

void ContactAddress::setPostcode(const QString &value)
{
    if (value != postcode()) {
        mDetail.setValue(QContactAddress::FieldPostcode, value);
        emit changed();
    }
}

QString ContactAddress::region() const
{
    return mDetail.value(QContactAddress::FieldRegion);
}

void ContactAddress::setRegion(const QString &value)
{
    if (value != region()) {
        mDetail.setValue(QContactAddress::FieldRegion, value);
        emit changed();
    }
}

QString ContactAddress::street() const
{
    return mDetail.value(QContactAddress::FieldStreet);
}

void ContactAddress::setStreet(const QString &value)
{
    if (value != street()) {
        mDetail.setValue(QContactAddress::FieldStreet, value);
        emit changed();
    }
}

QStringList ContactAddress::subTypes() const
{
    return mDetail.value<QStringList>(QContactAddress::FieldSubTypes);
}

void ContactAddress::setSubTypes(const QStringList &value)
{
    //FIXME: we are just setting this one all the time, maybe we should check if the list really changed
    mDetail.setValue(QContactAddress::FieldSubTypes, value);
    emit changed();
}
