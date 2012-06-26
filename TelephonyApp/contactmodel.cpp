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
#include "contactmanager.h"
#include <QContactDetailFilter>
#include <QContactGuid>
#include <QContactSaveRequest>
#include <QContactPhoneNumber>
#include <QDebug>
#include <QUrl>

ContactModel::ContactModel(QObject *parent) :
    QAbstractListModel(parent), mContactManager(ContactManager::instance())
{
    QHash<int, QByteArray> roles = roleNames();
    roles[ContactRole] = "contact";
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

ContactEntry *ContactModel::contactFromPhoneNumber(const QString &phoneNumber)
{
    QContact contact = mContactManager->contactForNumber(phoneNumber);
    return contactFromId(contact.detail<QContactGuid>().guid());
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
            SLOT(onContactsSaved()));

    request->start();
}

void ContactModel::loadContactFromId(const QString &guid)
{
    // check if the contact is already there
    ContactEntry *entry = contactFromId(guid);
    if (entry) {
        emit contactLoaded(entry);
    } else {
        // if it is not, save the guid for when it loads
        mPendingId = guid;
    }
}

void ContactModel::addContacts(const QList<QContact> &contacts)
{
    beginInsertRows(QModelIndex(), mContactEntries.count(), mContactEntries.count()+contacts.count()-1);

    Q_FOREACH(const QContact &contact, contacts) {
        ContactEntry *entry = new ContactEntry(contact, this);
        connect(entry,
                SIGNAL(changed(ContactEntry*)),
                SLOT(onContactEntryChanged(ContactEntry*)));
        mContactEntries.append(entry);
        if (entry->id() == mPendingId) {
            emit contactLoaded(entry);
            mPendingId = "";
        }
    }

    endInsertRows();
}

void ContactModel::removeContact(ContactEntry *entry)
{
    int index = mContactEntries.indexOf(entry);
    if (index < 0) {
        qWarning() << "Trying to remove an entry that is not in the model:" << entry;
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    mContactEntries.removeAt(index);
    endRemoveRows();
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
        if (ids.indexOf(entry->localId() >= 0)) {
            removeContact(entry);
        }
    }
}

void ContactModel::onContactEntryChanged(ContactEntry *entry)
{
    QModelIndex entryIndex = index(mContactEntries.indexOf(entry), 0);
    emit dataChanged(entryIndex, entryIndex);
}

void ContactModel::onContactSaved()
{
    QContactSaveRequest *request = qobject_cast<QContactSaveRequest*>(QObject::sender());
    if (request->isFinished() && request->error() != QContactManager::NoError) {
        qWarning() << "Failed to save the contact. Error:" << request->error();
        //FIXME: maybe we should map the error codes to texts
    }

    // there is no need to process the result of the request as we are watching the contacts added,
    // removed and changed signals
}
