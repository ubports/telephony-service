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

#include "contactmanager.h"
#include <QContactDetailFilter>
#include <QContactPhoneNumber>

ContactManager::ContactManager() :
    QContactManager("folks")
{
}

ContactManager *ContactManager::instance()
{
    static ContactManager* manager = new ContactManager();
    return manager;
}

QContact ContactManager::contactForNumber(const QString &number)
{
    // fetch the QContact object
    QContactDetailFilter filter;
    filter.setDetailDefinitionName(QContactPhoneNumber::DefinitionName, QContactPhoneNumber::FieldNumber);
    filter.setValue(number);
    filter.setMatchFlags(QContactFilter::MatchPhoneNumber);

    QList<QContact> contactList = contacts(filter);
    if (contactList.count() > 0) {
        return contactList[0];
    }
    return QContact();
}
