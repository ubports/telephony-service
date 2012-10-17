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

#ifndef CONTACTEMAILADDRESS_H
#define CONTACTEMAILADDRESS_H

#include "contactdetail.h"
#include <QContactEmailAddress>

class ContactEmailAddress : public ContactDetail
{
    Q_OBJECT
    Q_PROPERTY(QString emailAddress
               READ emailAddress
               WRITE setEmailAddress
               NOTIFY changed)
public:
    explicit ContactEmailAddress(const QContactDetail &detail = QContactEmailAddress(), QObject *parent = 0);
    
    QString emailAddress() const;
    void setEmailAddress(const QString &value);

Q_SIGNALS:
    void changed();
    
};

#endif // CONTACTEMAILADDRESS_H
