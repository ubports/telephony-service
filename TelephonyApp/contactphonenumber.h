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

#ifndef CONTACTPHONENUMBER_H
#define CONTACTPHONENUMBER_H

#include "contactdetail.h"
#include <QContactPhoneNumber>

class ContactPhoneNumber : public ContactDetail
{
    Q_OBJECT
    Q_PROPERTY(QString number
               READ number
               WRITE setNumber
               NOTIFY numberChanged)
    Q_PROPERTY(QVariant subTypes
               READ subTypes
               WRITE setSubTypes
               NOTIFY subTypesChanged)
public:
    explicit ContactPhoneNumber(const QContactDetail &detail = QContactPhoneNumber(), QObject *parent = 0);
    
    int type() const;

    QString number() const;
    void setNumber(const QString &value);

    QVariant subTypes() const;
    void setSubTypes(const QVariant &value);

Q_SIGNALS:
    void changed();
    void numberChanged();
    void subTypesChanged();
    
    
};

#endif // CONTACTPHONENUMBER_H
