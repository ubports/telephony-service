/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "contactmodel.h"
#include "contactentry.h"
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
#include <QVersitContactImporter>
#include <QVersitReader>
#include "config.h"

QTVERSIT_USE_NAMESPACE

ContactModel *ContactModel::instance(const QString &engine)
{
    static ContactModel *model = new ContactModel(engine);
    return model;
}

QContactManager *ContactModel::contactManager()
{
    return mContactManager;
}

ContactModel::ContactModel(const QString &engine, QObject *parent) :
    QAbstractListModel(parent)
{
    QString availableEngine = engine;
    bool useFakeData = false;
    if (!QContactManager::availableManagers().contains(engine)) {
        qDebug() << "ContactModel: using memory engine as folks is not available";
        availableEngine = "memory";
        useFakeData = true;
    }

    mContactManager = new QContactManager(availableEngine);
    if (useFakeData) {
        // load some fake contacts to have the list populated
        QFile file(telephonyAppDirectory() + "/dummydata/example.vcf");
        if (file.open(QIODevice::ReadOnly)) {
            QVersitReader reader(&file);
            reader.startReading();
            reader.waitForFinished();

            QVersitContactImporter importer;
            if (importer.importDocuments(reader.results())) {
                QList<QContact> contacts = importer.contacts();
                mContactManager->saveContacts(&contacts);
            }
        }
    }

    QHash<int, QByteArray> roles = roleNames();
    roles[ContactRole] = "contact";
    roles[InitialRole] = "initial";
    setRoleNames(roles);

    connect(mContactManager,
            SIGNAL(contactsAdded(QList<QContactId>)),
            SLOT(onContactsAdded(QList<QContactId>)));
    connect(mContactManager,
            SIGNAL(contactsChanged(QList<QContactId>)),
            SLOT(onContactsChanged(QList<QContactId>)));
    connect(mContactManager,
            SIGNAL(contactsRemoved(QList<QContactId>)),
            SLOT(onContactsRemoved(QList<QContactId>)));

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

ContactEntry *ContactModel::contactFromId(const QString &id)
{
    Q_FOREACH(ContactEntry *entry, mContactEntries) {
        if (entry->id().toString() == id) {
            return entry;
        }
    }

    return 0;
}

ContactEntry *ContactModel::contactFromPhoneNumber(const QString &phoneNumber)
{
    Q_FOREACH(ContactEntry *entry, mContactEntries) {
        Q_FOREACH(const QContactPhoneNumber &storedPhoneNumber, entry->contact().details<QContactPhoneNumber>()) {
            if (comparePhoneNumbers(storedPhoneNumber.number(), phoneNumber)) {
                return entry;
            }
        }
    }

    return 0;
}

void ContactModel::saveContact(ContactEntry *entry)
{
    if (!entry->modified()) {
        return;
    }

    QContactSaveRequest *request = new QContactSaveRequest(this);
    request->setManager(mContactManager);
    request->setContact(entry->contact());

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
    request->setContactId(entry->id());

    connect(request,
            SIGNAL(stateChanged(QContactAbstractRequest::State)),
            SLOT(onContactRemoved()));

    request->start();
}

void ContactModel::updateContact(ContactEntry *entry)
{
    entry->setContact(mContactManager->contact(entry->id()));
}

void ContactModel::addContacts(const QList<QContact> &contacts)
{
    if (!contacts.count()) {
        return;
    }

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
        if (entry->id().toString() == mPendingId) {
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
    Q_EMIT contactRemoved(entry->id().toString());
}

void ContactModel::onContactsAdded(QList<QContactId> ids)
{
    addContacts(mContactManager->contacts(ids));
}

void ContactModel::onContactsChanged(QList<QContactId> ids)
{
    Q_FOREACH(ContactEntry *entry, mContactEntries) {
        if (ids.contains(entry->id())) {
            // the changed signal is going to be emitted by the entry
            entry->setContact(mContactManager->contact(entry->id()));
        }
    }
}

void ContactModel::onContactsRemoved(QList<QContactId> ids)
{
    Q_FOREACH(ContactEntry *entry, mContactEntries) {
        if (ids.contains(entry->id())) {
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
            QString id = contact.id().toString();
            Q_EMIT contactSaved(id);
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

bool ContactModel::comparePhoneNumbers(const QString &number1, const QString &number2)
{
    return PhoneNumberUtils::compareLoosely(number1, number2);
}
