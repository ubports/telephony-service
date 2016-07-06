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

#include "contactutils.h"
#include <QContactName>
#include <QContactDisplayLabel>

QTCONTACTS_USE_NAMESPACE

namespace ContactUtils
{

QContactManager *sharedManager(const QString &engine)
{
    QString finalEngine = engine;
    if (!qgetenv("TELEPHONY_SERVICE_TEST").isEmpty()) {
        finalEngine = "memory";
    }
    static QContactManager *instance = new QContactManager(finalEngine);
    return instance;
}

// Note: update GreeterContacts::mapToContact() if this function is modified
// to use more than just first and last names.
QString formatContactName(const QContact &contact)
{
    // try contact display label
    QContactDisplayLabel displayLabel = contact.detail<QContactDisplayLabel>();
    if (!displayLabel.isEmpty() && !displayLabel.label().isEmpty()) {
        return displayLabel.label();
    }

    // if empty fallback to contact name
    QContactName name = contact.detail<QContactName>();
    QString formattedName = name.firstName();

    if (!name.middleName().isEmpty()) {
        formattedName += " " + name.middleName();
    }

    if (!name.lastName().isEmpty()) {
        formattedName += " " + name.lastName();
    }

    return formattedName.trimmed();
}

}
