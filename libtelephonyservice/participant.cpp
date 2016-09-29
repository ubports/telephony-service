/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "participant.h"

Participant::Participant(const QString &identifier, uint roles, uint handle, QObject *parent)
: ContactWatcher(parent), mRoles(roles), mHandle(handle)
{
    classBegin();
    setIdentifier(identifier);
    componentComplete();
}

Participant::Participant(QObject *parent)
: ContactWatcher(parent)
{
    classBegin();
    componentComplete();
}

Participant::Participant(const Participant &other)
{
    // we just need to set the identifier, the rest will come after the info is fetched
    setIdentifier(other.identifier());
}

Participant::~Participant()
{
}

void Participant::setRoles(uint roles)
{
    mRoles = roles;
}

uint Participant::roles() const
{
    return mRoles;
}

uint Participant::handle() const
{
    return mHandle;
}
