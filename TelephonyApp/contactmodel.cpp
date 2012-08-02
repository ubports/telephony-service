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

#include "contactmodel.h"
#include "contactentry.h"
#include "contactcustomid.h"
#include "phoneutils.h"
#include <QContactDetailFilter>
#include <QContactGuid>
#include <QContactSaveRequest>
#include <QContactRemoveRequest>
#include <QContactPhoneNumber>
#include <QDebug>
#include <QUrl>
#include <QDBusInterface>
#include <QDBusReply>

ContactModel *ContactModel::instance()
{
    static ContactModel *model = new ContactModel();
    return model;
}

ContactModel::ContactModel(QObject *parent) :
    QAbstractListModel(parent), mContactManager(new QContactManager("folks"))
{
    QHash<int, QByteArray> roles = roleNames();
    roles[ContactRole] = "contact";
    roles[InitialRole] = "initial";
    setRoleNames(roles);

    connect(mContactManager,
            SIGNAL(contactsAdded(QList<QContactLocalId>)),
            SLOT(onContactsAdded(QList<QContactLocalId>)));
    connect(mContactManager,
            SIGNAL(contactsChanged(QList<QContactLocalId>)),
            SLOT(onContactsChanged(QList<QContactLocalId>)));
    connect(mContactManager,
            SIGNAL(contactsRemoved(QList<QContactLocalId>)),
            SLOT(onContactsRemoved(QList<QContactLocalId>)));

    addContacts(mContactManager->contacts());
}

int ContactModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return mContactEntries.count();
}

QVariant ContactModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= mContactEntries.count()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return mContactEntries[index.row()]->displayLabel();
    case Qt::DecorationRole:
        return mContactEntries[index.row()]->avatar();
    case ContactRole:
        return QVariant::fromValue(const_cast<QObject *>(static_cast<const QObject *>(mContactEntries[index.row()])));
    case InitialRole:
        return mContactEntries[index.row()]->initial();
    }

    return QVariant();
}

ContactEntry *ContactModel::contactFromId(const QString &guid)
{
    Q_FOREACH(ContactEntry *entry, mContactEntries) {
        if (entry->id() == guid) {
            return entry;
        }
    }

    return 0;
}

ContactEntry *ContactModel::contactFromCustomId(const QString &customId)
{
    if (customId.isEmpty()) {
        return 0;
    }

    Q_FOREACH(ContactEntry *entry, mContactEntries) {
        if (entry->customId() == customId) {
            return entry;
        }
    }

    return 0;
}

QString ContactModel::customIdFromPhoneNumber(const QString &phoneNumber)
{
    // FIXME: replace this by something not relying specifically on android
    QDBusInterface contacts("com.canonical.Android",
                            "/com/canonical/android/contacts/Contacts",
                            "com.canonical.android.contacts.Contacts");
    QDBusReply<QString> reply = contacts.call("getContactKeyForNumber", phoneNumber);
    QString id = reply.value();
    return id;

}

ContactEntry *ContactModel::contactFromPhoneNumber(const QString &phoneNumber)
{
    QString id = customIdFromPhoneNumber(phoneNumber);
    if (id.isEmpty()) {
        return 0;
    }

    return contactFromCustomId(id);
}

void ContactModel::saveContact(ContactEntry *entry)
{
    if (!entry->modified()) {
        return;
    }

    QContact contact = mContactManager->compatibleContact(entry->contact());
    QContactSaveRequest *request = new QContactSaveRequest(this);
    request->setManager(mContactManager);
    request->setContact(contact);

    connect(request,
            SIGNAL(stateChanged(QContactAbstractRequest::State)),
            SLOT(onContactSaved()));

    request->start();
}

void ContactModel::loadContactFromId(const QString &guid)
{
    // check if the contact is already there
    ContactEntry *entry = contactFromId(guid);
    if (entry) {
        Q_EMIT contactLoaded(entry);
    } else {
        // if it is not, save the guid for when it loads
        mPendingId = guid;
    }
}

void ContactModel::removeContact(ContactEntry *entry)
{
    QContactRemoveRequest *request = new QContactRemoveRequest(this);
    request->setManager(mContactManager);
    request->setContactId(entry->localId());

    connect(request,
            SIGNAL(stateChanged(QContactAbstractRequest::State)),
            SLOT(onContactRemoved()));

    request->start();
}

void ContactModel::updateContact(ContactEntry *entry)
{
    entry->setContact(mContactManager->contact(entry->localId()));
}

void ContactModel::addContacts(const QList<QContact> &contacts)
{
    ContactEntry *pending = 0;

    beginInsertRows(QModelIndex(), mContactEntries.count(), mContactEntries.count()+contacts.count()-1);

    Q_FOREACH(const QContact &contact, contacts) {
        ContactEntry *entry = new ContactEntry(contact, this);
        connect(entry,
                SIGNAL(changed(ContactEntry*)),
                SLOT(onContactEntryChanged(ContactEntry*)));
        mContactEntries.append(entry);
        Q_EMIT contactAdded(entry);

        // check if this entry is pending load
        if (entry->id() == mPendingId) {
            pending = entry;
        }
    }

    endInsertRows();

    if (pending != 0) {
        Q_EMIT contactLoaded(pending);
        mPendingId = "";
    }
}

void ContactModel::removeContactFromModel(ContactEntry *entry)
{
    int index = mContactEntries.indexOf(entry);
    if (index < 0) {
        qWarning() << "Trying to remove an entry that is not in the model:" << entry;
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    mContactEntries.removeAt(index);
    entry->deleteLater();
    endRemoveRows();
    Q_EMIT contactRemoved(entry->customId());
}

void ContactModel::onContactsAdded(QList<QContactLocalId> ids)
{
    addContacts(mContactManager->contacts(ids));
}

void ContactModel::onContactsChanged(QList<QContactLocalId> ids)
{
    Q_FOREACH(ContactEntry *entry, mContactEntries) {
        if (ids.contains(entry->localId())) {
            // the changed signal is going to be emitted by the entry
            entry->setContact(mContactManager->contact(entry->localId()));
        }
    }
}

void ContactModel::onContactsRemoved(QList<QContactLocalId> ids)
{
    Q_FOREACH(ContactEntry *entry, mContactEntries) {
        if (ids.contains(entry->localId())) {
            removeContactFromModel(entry);
        }
    }
}

void ContactModel::onContactEntryChanged(ContactEntry *entry)
{
    QModelIndex entryIndex = index(mContactEntries.indexOf(entry), 0);
    Q_EMIT contactChanged(entry);
    Q_EMIT dataChanged(entryIndex, entryIndex);
}

void ContactModel::onContactSaved()
{
    QContactSaveRequest *request = qobject_cast<QContactSaveRequest*>(QObject::sender());
    if (request->isFinished()) {
        if (request->error() != QContactManager::NoError) {
            qWarning() << "Failed to save the contact. Error:" << request->error();
            //FIXME: maybe we should map the error codes to texts
        } else {
            // each request contains just one contact as we just ask one contact to be saved at a time
            QContact contact = request->contacts().first();
            QString id = contact.detail<QContactGuid>().guid();
            QString customId = contact.detail<ContactCustomId>().customId().split(":").last();
            Q_EMIT contactSaved(id, customId);
        }
    }
}

void ContactModel::onContactRemoved()
{
    QContactRemoveRequest *request = qobject_cast<QContactRemoveRequest*>(QObject::sender());
    if (request->isFinished() && request->error() != QContactManager::NoError) {
        qWarning() << "Failed to remove the contact. Error:" << request->error();
        //FIXME: maybe we should map the error codes to texts
    }

    // there is no need to process the result of the request as we are watching the contacts added,
    // removed and changed signals
}

bool ContactModel::comparePhoneNumbers(const QString &number1, const QString &number2) const
{
    return PhoneNumberUtils::compareLoosely(number1, number2);
}
