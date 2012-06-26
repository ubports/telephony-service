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

#ifndef CONTACTONLINEACCOUNT_H
#define CONTACTONLINEACCOUNT_H

#include "contactdetail.h"
#include <QContactOnlineAccount>

class ContactOnlineAccount : public ContactDetail
{
    Q_OBJECT
    Q_PROPERTY(QString accountUri READ accountUri WRITE setAccountUri NOTIFY changed)
    Q_PROPERTY(QStringList capabilities READ capabilities WRITE setCapabilities NOTIFY changed)
    Q_PROPERTY(QString protocol READ protocol WRITE setProtocol NOTIFY changed)
    Q_PROPERTY(QStringList subTypes READ subTypes WRITE setSubTypes NOTIFY changed)

public:
    explicit ContactOnlineAccount(const QContactDetail &detail = QContactOnlineAccount(), QObject *parent = 0);

    int type() const;

    QString accountUri() const;
    void setAccountUri(const QString &value);

    QStringList capabilities() const;
    void setCapabilities(const QStringList &value);

    QString protocol() const;
    void setProtocol(const QString &value);

    QString serviceProvider() const;
    void setServiceProvider(const QString &value);

    QStringList subTypes() const;
    void setSubTypes(const QStringList &value);
    
Q_SIGNALS:
    void changed();
    
};

#endif // CONTACTONLINEACCOUNT_H
