/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "contactaddress.h"

ContactAddress::ContactAddress(const QContactDetail &detail, QObject *parent) :
    ContactDetail(detail, parent)
{
    connect(this,
            SIGNAL(detailChanged()),
            SIGNAL(changed()));
}

QString ContactAddress::country() const
{
    return mDetail.value(QContactAddress::FieldCountry).toString();
}

void ContactAddress::setCountry(const QString &value)
{
    if (value != country()) {
        mDetail.setValue(QContactAddress::FieldCountry, value);
        Q_EMIT changed();
        Q_EMIT countryChanged();
    }
}

QString ContactAddress::locality() const
{
    return mDetail.value(QContactAddress::FieldLocality).toString();
}

void ContactAddress::setLocality(const QString &value)
{
    if (value != locality()) {
        mDetail.setValue(QContactAddress::FieldLocality, value);
        Q_EMIT changed();
        Q_EMIT localityChanged();
    }
}

QString ContactAddress::postOfficeBox() const
{
    return mDetail.value(QContactAddress::FieldPostOfficeBox).toString();
}

void ContactAddress::setPostOfficeBox(const QString &value)
{
    if (value != postOfficeBox()) {
        mDetail.setValue(QContactAddress::FieldPostOfficeBox, value);
        Q_EMIT changed();
        Q_EMIT postOfficeBoxChanged();
    }
}

QString ContactAddress::postcode() const
{
    return mDetail.value(QContactAddress::FieldPostcode).toString();
}

void ContactAddress::setPostcode(const QString &value)
{
    if (value != postcode()) {
        mDetail.setValue(QContactAddress::FieldPostcode, value);
        Q_EMIT changed();
        Q_EMIT postcodeChanged();
    }
}

QString ContactAddress::region() const
{
    return mDetail.value(QContactAddress::FieldRegion).toString();
}

void ContactAddress::setRegion(const QString &value)
{
    if (value != region()) {
        mDetail.setValue(QContactAddress::FieldRegion, value);
        Q_EMIT changed();
        Q_EMIT regionChanged();
    }
}

QString ContactAddress::street() const
{
    return mDetail.value(QContactAddress::FieldStreet).toString();
}

void ContactAddress::setStreet(const QString &value)
{
    if (value != street()) {
        mDetail.setValue(QContactAddress::FieldStreet, value);
        Q_EMIT changed();
        Q_EMIT streetChanged();
    }
}

QVariant ContactAddress::subTypes() const
{
    return mDetail.value<QVariant>(QContactAddress::FieldSubTypes);
}

void ContactAddress::setSubTypes(const QVariant &value)
{
    mDetail.setValue(QContactAddress::FieldSubTypes,  QVariant::fromValue(intListFromVariant(value)));
    Q_EMIT changed();
    Q_EMIT subTypesChanged();
}
