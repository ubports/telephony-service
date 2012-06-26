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
#include <QDeclarativeListProperty>
#include <QUrl>
#include "contactdetail.h"

using namespace QtMobility;

class ContactName;
class ContactPhoneNumber;

class ContactEntry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id
               READ id
               NOTIFY changed)
    Q_PROPERTY(QString displayLabel
               READ displayLabel
               NOTIFY changed)
    Q_PROPERTY(QUrl avatar
               READ avatar
               NOTIFY changed)
    Q_PROPERTY(ContactName *name
               READ name
               NOTIFY changed)
    Q_PROPERTY(QDeclarativeListProperty<ContactDetail> phoneNumbers
               READ phoneNumbers
               NOTIFY changed)
    Q_PROPERTY(QDeclarativeListProperty<ContactDetail> emails
               READ emails
               NOTIFY changed)

public:
    explicit ContactEntry(const QContact &contact = QContact(), QObject *parent = 0);

    QContactLocalId localId() const;
    QString id() const;

    QString displayLabel() const;
    QUrl avatar() const;
    ContactName *name() const;

    void setContact(const QContact &contact);

    QDeclarativeListProperty<ContactDetail> phoneNumbers();
    QDeclarativeListProperty<ContactDetail> emails();
    
    // QDeclarativeListProperty helpers
    static void detailAppend(QDeclarativeListProperty<ContactDetail> *p, ContactDetail *detail);
    static int  detailCount(QDeclarativeListProperty<ContactDetail> *p);
    static ContactDetail* detailAt(QDeclarativeListProperty<ContactDetail> *p, int index);
    //static void  detailClear(QDeclarativeListProperty<ContactDetail*> *p);

Q_SIGNALS:
    void changed(ContactEntry *entry);

protected:
    void loadDetails();

protected Q_SLOTS:
    void onDetailChanged();

private:
    QContact mContact;
    ContactName *mName;
    QMap<ContactDetail::DetailType, QList<ContactDetail*> > mDetails;
};

#endif // CONTACTENTRY_H
