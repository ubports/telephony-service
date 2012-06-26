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

#include "contactemailaddress.h"
#include "contactentry.h"
#include "contactname.h"
#include "contactphonenumber.h"
#include <QContactGuid>
#include <QContactAvatar>
#include <QContactEmailAddress>
#include <QContactPhoneNumber>
#include <QDebug>

ContactEntry::ContactEntry(const QContact &contact, QObject *parent) :
    QObject(parent), mContact(contact)
{
    mName = new ContactName(contact.detail<QContactName>(), this);
    connect(mName,
            SIGNAL(changed()),
            SLOT(onDetailChanged()));

    loadDetails();
}

QContactLocalId ContactEntry::localId() const
{
    return mContact.localId();
}

QString ContactEntry::id() const
{
    return mContact.detail<QContactGuid>().guid();
}

QString ContactEntry::displayLabel() const
{
    return mName->customLabel();
}

QUrl ContactEntry::avatar() const
{
    return mContact.detail<QContactAvatar>().imageUrl();
}

ContactName *ContactEntry::name() const
{
    return mName;
}

void ContactEntry::setContact(const QContact &contact)
{
    mContact = contact;
    emit changed(this);
}

QDeclarativeListProperty<ContactDetail> ContactEntry::phoneNumbers()
{
    static int type = ContactDetail::PhoneNumber;
    return QDeclarativeListProperty<ContactDetail>(this, (void*) &type, detailAppend, detailCount, detailAt);
}

QDeclarativeListProperty<ContactDetail> ContactEntry::emails()
{
    static int type = ContactDetail::EmailAddress;
    return QDeclarativeListProperty<ContactDetail>(this, (void*) &type, detailAppend, detailCount, detailAt);
}

void ContactEntry::onDetailChanged()
{
    emit changed(this);
}

void ContactEntry::detailAppend(QDeclarativeListProperty<ContactDetail> *p, ContactDetail *detail)
{
    ContactEntry *entry = qobject_cast<ContactEntry*>(p->object);
    if (!entry) {
        qWarning() << "Object is not a ContactEntry!";
        return;
    }

    int type = *(int*)p->data;
    // FIXME: check if we shouldn't copy the detail instead of just appending it
    entry->mDetails[(ContactDetail::DetailType)type].append(detail);
    // FIXME: add the detail to the contact
}

int ContactEntry::detailCount(QDeclarativeListProperty<ContactDetail> *p)
{
    ContactEntry *entry = qobject_cast<ContactEntry*>(p->object);
    if (!entry) {
        qWarning() << "Object is not a ContactEntry!";
        return 0;
    }

    int type = *(int*)p->data;
    return entry->mDetails[(ContactDetail::DetailType)type].count();

}

ContactDetail *ContactEntry::detailAt(QDeclarativeListProperty<ContactDetail> *p, int index)
{
    ContactEntry *entry = qobject_cast<ContactEntry*>(p->object);
    if (!entry) {
        qWarning() << "Object is not a ContactEntry!";
        return 0;
    }

    int type = *(int*)p->data;

    return entry->mDetails[(ContactDetail::DetailType)type].at(index);
}

void ContactEntry::loadDetails()
{
    // Phone numbers
    Q_FOREACH(const QContactPhoneNumber &detail, mContact.details<QContactPhoneNumber>()) {
        ContactPhoneNumber *number = new ContactPhoneNumber(detail, this);
        connect(number,
                SIGNAL(changed()),
                SLOT(onDetailChanged()));
        mDetails[ContactDetail::PhoneNumber].append(number);
    }

    // Email addresses
    Q_FOREACH(const QContactEmailAddress &detail, mContact.details<QContactEmailAddress>()) {
        ContactEmailAddress *email = new ContactEmailAddress(detail, this);
        connect(email,
                SIGNAL(changed()),
                SLOT(onDetailChanged()));
        mDetails[ContactDetail::EmailAddress].append(email);
    }
}

/*void ContactEntry::detailClear(QDeclarativeListProperty<ContactDetail*> *p)
{
    ContactEntry *entry = qobject_cast<ContactEntry*>(p->object);
    if (!entry) {
        qWarning() << "Object is not a ContactEntry!";
        return;
    }

    int type = *(int*)p->data;
}*/
