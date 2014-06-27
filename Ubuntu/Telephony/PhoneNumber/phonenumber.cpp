/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "phonenumber.h"
#include "asyoutypeformatter.h"

#include <QQmlEngine>
#include <qqml.h>

void PhoneNumber::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(engine);
    Q_UNUSED(uri);
}

void PhoneNumber::registerTypes(const char *uri)
{
    // @uri Telephony.PhoneNumber
    qmlRegisterType<AsYouTypeFormatter>(uri, 0, 1, "AsYouTypeFormatter");
}
