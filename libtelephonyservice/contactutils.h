/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONTACTUTILS_H
#define CONTACTUTILS_H

#include <QContactManager>
#include <QContact>

#define OFONO_UNKNOWN_NUMBER "x-ofono-unknown"
#define OFONO_PRIVATE_NUMBER "x-ofono-private"

QTCONTACTS_USE_NAMESPACE

namespace ContactUtils
{
    QContactManager *sharedManager(const QString &engine = "org.nemomobile.contacts.sqlite");
    QString formatContactName(const QContact &contact);
}

#endif // CONTACTUTILS_H
