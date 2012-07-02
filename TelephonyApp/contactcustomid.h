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

#ifndef CONTACTCUSTOMID_H
#define CONTACTCUSTOMID_H

#include <QContactDetail>
#include <QLatin1Constant>

using namespace QtMobility;

class Q_CONTACTS_EXPORT ContactCustomId : public QContactDetail
{
public:
    Q_DECLARE_CUSTOM_CONTACT_DETAIL(ContactCustomId, "CustomId")
    Q_DECLARE_LATIN1_CONSTANT(FieldCustomId, "CustomId");

    void setCustomId(const QString& id);
    QString customId() const;
};

#endif // CONTACTCUSTOMID_H
