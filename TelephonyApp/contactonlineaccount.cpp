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

#include "contactonlineaccount.h"

ContactOnlineAccount::ContactOnlineAccount(const QContactDetail &detail, QObject *parent) :
    ContactDetail(detail, parent)
{
    connect(this,
            SIGNAL(detailChanged()),
            SIGNAL(changed()));
}

int ContactOnlineAccount::type() const
{
    return InstantMessaging;
}

QString ContactOnlineAccount::accountUri() const
{
    return mDetail.value(QContactOnlineAccount::FieldAccountUri);
}

void ContactOnlineAccount::setAccountUri(const QString &value)
{
    if (value != accountUri()) {
        mDetail.setValue(QContactOnlineAccount::FieldAccountUri, value);
        emit changed();
    }
}

QVariant ContactOnlineAccount::capabilities() const
{
    return mDetail.value<QStringList>(QContactOnlineAccount::FieldCapabilities);
}

void ContactOnlineAccount::setCapabilities(const QVariant &value)
{
    // FIXME: maybe we should check if the list has really changed?
    mDetail.setValue(QContactOnlineAccount::FieldCapabilities, value);
    emit changed();
}

QString ContactOnlineAccount::protocol() const
{
    return mDetail.value(QContactOnlineAccount::FieldProtocol);
}

void ContactOnlineAccount::setProtocol(const QString &value)
{
    if (value != protocol()) {
        mDetail.setValue(QContactOnlineAccount::FieldProtocol, value);
        emit changed();
    }
}

QString ContactOnlineAccount::serviceProvider() const
{
    return mDetail.value(QContactOnlineAccount::FieldServiceProvider);
}

void ContactOnlineAccount::setServiceProvider(const QString &value)
{
    if (value != serviceProvider()) {
        mDetail.setValue(QContactOnlineAccount::FieldServiceProvider, value);
        emit changed();
    }
}

QVariant ContactOnlineAccount::subTypes() const
{
    return mDetail.value<QStringList>(QContactOnlineAccount::FieldSubTypes);
}

void ContactOnlineAccount::setSubTypes(const QVariant &value)
{
    // FIXME: maybe we should check if the list has really changed?
    mDetail.setValue(QContactOnlineAccount::FieldSubTypes, value);
    emit changed();
}
