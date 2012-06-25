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

#ifndef CONTACTENTRY_H
#define CONTACTENTRY_H

#include <QObject>
#include <QContact>

using namespace QtMobility;

class ContactEntry : public QObject
{
    Q_OBJECT
public:
    explicit ContactEntry(const QContact &contact = QContact(), QObject *parent = 0);

    QContactLocalId localId() const;
    QString id() const;

    QString displayLabel() const;
    QUrl avatar() const;

    void setContact(const QContact &contact);
    
Q_SIGNALS:
    void changed(ContactEntry *entry);

private:
    QContact mContact;
};

#endif // CONTACTENTRY_H
