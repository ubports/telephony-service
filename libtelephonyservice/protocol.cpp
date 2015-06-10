/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include "protocol.h"

Protocol::Protocol(const QString &name, Features features, const QString &fallbackProtocol, QObject *parent)
: QObject(parent), mName(name), mFeatures(features), mFallbackProtocol(fallbackProtocol)
{

}

QString Protocol::name() const
{
    return mName;
}

Protocol::Features Protocol::features() const
{
    return mFeatures;
}

QString Protocol::fallbackProtocol() const
{
    return mFallbackProtocol;
}
