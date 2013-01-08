/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONTACTONLINEACCOUNT_H
#define CONTACTONLINEACCOUNT_H

#include "contactdetail.h"
#include <QContactOnlineAccount>

class ContactOnlineAccount : public ContactDetail
{
    Q_OBJECT
    Q_PROPERTY(QString accountUri READ accountUri WRITE setAccountUri NOTIFY accountUriChanged)
    Q_PROPERTY(QVariant capabilities READ capabilities WRITE setCapabilities NOTIFY capabilitiesChanged)
    Q_PROPERTY(QString protocol READ protocol WRITE setProtocol NOTIFY protocolChanged)
    Q_PROPERTY(QString serviceProvider READ serviceProvider WRITE setServiceProvider NOTIFY serviceProviderChanged)
    Q_PROPERTY(QVariant subTypes READ subTypes WRITE setSubTypes NOTIFY subTypesChanged)

public:
    explicit ContactOnlineAccount(const QContactDetail &detail = QContactOnlineAccount(), QObject *parent = 0);

    QString accountUri() const;
    void setAccountUri(const QString &value);

    QVariant capabilities() const;
    void setCapabilities(const QVariant &value);

    QString protocol() const;
    void setProtocol(const QString &value);

    QString serviceProvider() const;
    void setServiceProvider(const QString &value);

    QVariant subTypes() const;
    void setSubTypes(const QVariant &value);
    
Q_SIGNALS:
    void changed();
    void accountUriChanged();
    void capabilitiesChanged();
    void protocolChanged();
    void serviceProviderChanged();
    void subTypesChanged();
    
};

#endif // CONTACTONLINEACCOUNT_H
