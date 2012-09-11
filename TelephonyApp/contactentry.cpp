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

#include "contactaddress.h"
#include "contactemailaddress.h"
#include "contactentry.h"
#include "contactname.h"
#include "contactonlineaccount.h"
#include "contactphonenumber.h"
#include "contactmodel.h"
#include <QContactGuid>
#include <QContactAddress>
#include <QContactAvatar>
#include <QContactEmailAddress>
#include <QContactOnlineAccount>
#include <QContactPhoneNumber>
#include <QDebug>
#include <QLocale>

ContactEntry::ContactEntry(const QContact &contact, ContactModel *parent) :
    QObject(parent), mModel(parent)
{
    setContact(contact);
}

ContactEntry::ContactEntry(const ContactEntry &other)
{
    setContact(other.contact());
    mModel = other.mModel;
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
    return name()->customLabel();
}

QString ContactEntry::initial() const
{
    QString label = displayLabel();
    if (label.isEmpty()) {
        return "";
    } else {
        QLocale locale;
        return locale.toUpper(label.left(1));
    }
}

QUrl ContactEntry::avatar() const
{
    return mContact.detail<QContactAvatar>().imageUrl();
}

ContactName *ContactEntry::name() const
{
    return qobject_cast<ContactName*>(mDetails[ContactDetail::Name].first());
}

bool ContactEntry::modified() const
{
    return mModified;
}

void ContactEntry::setModified(bool value)
{
    if (value != mModified) {
        mModified = value;
        Q_EMIT modifiedChanged();
    }
}

QContact ContactEntry::contact() const
{
    return mContact;
}

void ContactEntry::setContact(const QContact &contact)
{
    mContact = contact;

    // remove invisible details created by folks
    Q_FOREACH(QContactDetail det, contact.details<QContactOnlineAccount>()) {
        if (det.contexts().contains("VISIBLE=FALSE", Qt::CaseInsensitive)) {
            mContact.removeDetail(&det);
        }
    }

    setModified(false);
    loadDetails();

    Q_EMIT changed(this);
}

QDeclarativeListProperty<ContactDetail> ContactEntry::addresses()
{
    static int type = ContactDetail::Address;
    return QDeclarativeListProperty<ContactDetail>(this, (void*) &type, detailAppend, detailCount, detailAt);
}

QDeclarativeListProperty<ContactDetail> ContactEntry::emails()
{
    static int type = ContactDetail::EmailAddress;
    return QDeclarativeListProperty<ContactDetail>(this, (void*) &type, detailAppend, detailCount, detailAt);
}

QDeclarativeListProperty<ContactDetail> ContactEntry::onlineAccounts()
{
    static int type = ContactDetail::InstantMessaging;
    return QDeclarativeListProperty<ContactDetail>(this, (void*) &type, detailAppend, detailCount, detailAt);
}

QDeclarativeListProperty<ContactDetail> ContactEntry::phoneNumbers()
{
    static int type = ContactDetail::PhoneNumber;
    return QDeclarativeListProperty<ContactDetail>(this, (void*) &type, detailAppend, detailCount, detailAt);
}

bool ContactEntry::addDetail(ContactDetail *detail)
{
    ContactDetail::DetailType type = (ContactDetail::DetailType) detail->type();

    ContactDetail *newDetail;
    // copy the detail into a new instance
    switch (type) {
    case ContactDetail::Address:
        newDetail = new ContactAddress(detail->detail(), this);
        break;
    case ContactDetail::EmailAddress:
        newDetail = new ContactEmailAddress(detail->detail(), this);
        break;
    case ContactDetail::InstantMessaging:
        newDetail = new ContactOnlineAccount(detail->detail(), this);
        break;
    case ContactDetail::PhoneNumber:
        newDetail = new ContactPhoneNumber(detail->detail(), this);
        break;
    default:
        newDetail = new ContactDetail(detail->detail(), this);
    }

    if (mContact.saveDetail(&newDetail->detail())) {
        setModified(true);
        mDetails[type].append(newDetail);
        connect(newDetail,
                SIGNAL(changed()),
                SLOT(onDetailChanged()));
        return true;
    } else {
        qWarning() << "Failed to add new detail to contact";
        delete newDetail;
    }
    return false;
}

bool ContactEntry::removeDetail(ContactDetail *detail)
{
    if (mContact.removeDetail(&detail->detail())) {
        // Removing the detail from the contact is enough at this point.
        // The QML might still access the detail object while animating so
        // don't remove it here.
        setModified(true);
        return true;
    }
    return false;
}

void ContactEntry::revertChanges()
{
    if (mModel) {
        mModel->updateContact(this);
    }
}

void ContactEntry::onDetailChanged()
{
    ContactDetail *detail = qobject_cast<ContactDetail*>(sender());
    if (!detail) {
        qWarning() << "Detail changed emitted from an object that is not a detail";
    }
    if (mContact.saveDetail(&detail->detail())) {
        setModified(true);
    }
}

void ContactEntry::detailAppend(QDeclarativeListProperty<ContactDetail> *p, ContactDetail *detail)
{
    ContactEntry *entry = qobject_cast<ContactEntry*>(p->object);
    if (!entry) {
        qWarning() << "Object is not a ContactEntry!";
        return;
    }

    entry->addDetail(detail);
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
    // clear previously saved details
    Q_FOREACH(const QList<ContactDetail*> list, mDetails.values()) {
        qDeleteAll(list);
    }
    mDetails.clear();

    load<QContactAddress, ContactAddress>();
    load<QContactEmailAddress, ContactEmailAddress>();
    load<QContactOnlineAccount, ContactOnlineAccount>();
    load<QContactPhoneNumber, ContactPhoneNumber>();
    load<QContactName, ContactName>();


    // if the contact doesn't have a name detail, create it.
    if (mDetails[ContactDetail::Name].isEmpty()) {
        ContactName *contactName = new ContactName(QContactName(), this);
        connect(contactName,
                SIGNAL(changed()),
                SLOT(onDetailChanged()));
        mDetails[ContactDetail::Name].append(contactName);
    }
}
