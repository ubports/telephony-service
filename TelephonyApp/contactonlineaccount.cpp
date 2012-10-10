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
    return mDetail.value(QContactOnlineAccount::FieldAccountUri).toString();
}

void ContactOnlineAccount::setAccountUri(const QString &value)
{
    if (value != accountUri()) {
        mDetail.setValue(QContactOnlineAccount::FieldAccountUri, value);
        Q_EMIT changed();
        Q_EMIT accountUriChanged();
    }
}

QVariant ContactOnlineAccount::capabilities() const
{
    return mDetail.value<QStringList>(QContactOnlineAccount::FieldCapabilities);
}

void ContactOnlineAccount::setCapabilities(const QVariant &value)
{
    // FIXME: maybe we should check if the list has really changed?
    mDetail.setValue(QContactOnlineAccount::FieldCapabilities, value.toStringList());
    Q_EMIT changed();
    Q_EMIT capabilitiesChanged();
}

QString ContactOnlineAccount::protocol() const
{
    // FIXME: onlineAccount protocols are not strings anymore in Qt5, port this code
    //return mDetail.value(QContactOnlineAccount::FieldProtocol).toString();
    return QString();
}

void ContactOnlineAccount::setProtocol(const QString &value)
{
    // FIXME: onlineAccount protocols are not strings anymore in Qt5, port this code
#if 0
    if (value != protocol()) {
        mDetail.setValue(QContactOnlineAccount::FieldProtocol, value);
        Q_EMIT changed();
        Q_EMIT protocolChanged();
    }
#endif
}

QString ContactOnlineAccount::serviceProvider() const
{
    return mDetail.value(QContactOnlineAccount::FieldServiceProvider).toString();
}

void ContactOnlineAccount::setServiceProvider(const QString &value)
{
    if (value != serviceProvider()) {
        mDetail.setValue(QContactOnlineAccount::FieldServiceProvider, value);
        Q_EMIT changed();
        Q_EMIT serviceProviderChanged();
    }
}

QVariant ContactOnlineAccount::subTypes() const
{
    return mDetail.value<QStringList>(QContactOnlineAccount::FieldSubTypes);
}

void ContactOnlineAccount::setSubTypes(const QVariant &value)
{
    // FIXME: maybe we should check if the list has really changed?
    mDetail.setValue(QContactOnlineAccount::FieldSubTypes, value.toStringList());
    Q_EMIT changed();
    Q_EMIT subTypesChanged();
}
