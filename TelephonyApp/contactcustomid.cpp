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

#include "contactcustomid.h"

Q_DEFINE_LATIN1_CONSTANT(ContactCustomId::DefinitionName, "CustomId");
Q_DEFINE_LATIN1_CONSTANT(ContactCustomId::FieldCustomId, "CustomId");

void ContactCustomId::setCustomId(const QString &id)
{
    setValue(FieldCustomId, id);
}

QString ContactCustomId::customId() const
{
    return value(FieldCustomId);
}
