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

#ifndef CONTACTADDRESS_H
#define CONTACTADDRESS_H

#include "contactdetail.h"
#include <QContactAddress>

class ContactAddress : public ContactDetail
{
    Q_OBJECT
    Q_PROPERTY(QString country READ country WRITE setCountry NOTIFY changed)
    Q_PROPERTY(QString locality READ locality WRITE setLocality NOTIFY changed)
    Q_PROPERTY(QString postOfficeBox READ postOfficeBox WRITE setPostOfficeBox NOTIFY changed)
    Q_PROPERTY(QString postcode READ postcode WRITE setPostcode NOTIFY changed)
    Q_PROPERTY(QString region READ region WRITE setRegion NOTIFY changed)
    Q_PROPERTY(QString street READ street WRITE setStreet NOTIFY changed)
    Q_PROPERTY(QStringList subTypes READ subTypes WRITE setSubTypes NOTIFY changed)

public:
    explicit ContactAddress(const QContactDetail &detail = QContactAddress(), QObject *parent = 0);

    int type() const;

    QString country() const;
    void setCountry(const QString &value);

    QString locality() const;
    void setLocality(const QString &value);

    QString postOfficeBox() const;
    void setPostOfficeBox(const QString &value);

    QString postcode() const;
    void setPostcode(const QString &value);

    QString region() const;
    void setRegion(const QString &value);

    QString street() const;
    void setStreet(const QString &value);

    QStringList subTypes() const;
    void setSubTypes(const QStringList &value);

Q_SIGNALS:
    void changed();

};

#endif // CONTACTADDRESS_H
