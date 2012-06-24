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

#ifndef CONTACTMANAGER_H
#define CONTACTMANAGER_H

#include <QContactManager>

using namespace QtMobility;

class ContactManager : public QContactManager
{
    Q_OBJECT
public:
    static ContactManager *instance();

    QContact contactForNumber(const QString &number);

private:
    explicit ContactManager();
    
};

#endif // CONTACTMANAGER_H
