/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "conversationfeedmodel.h"
#include "contactentry.h"
#include "contactmodel.h"
#include <QContactPhoneNumber>

namespace C {
#include <libintl.h>
}


ConversationFeedModel::ConversationFeedModel(QObject *parent) :
    QAbstractListModel(parent)
{
    connect(ContactModel::instance(),
            SIGNAL(contactAdded(ContactEntry*)),
            SLOT(onContactAdded(ContactEntry*)));
    connect(ContactModel::instance(),
            SIGNAL(contactChanged(ContactEntry*)),
            SLOT(onContactChanged(ContactEntry*)));
    connect(ContactModel::instance(),
            SIGNAL(contactRemoved(const QString&)),
            SLOT(onContactRemoved(const QString&)));
}

ConversationFeedModel::~ConversationFeedModel()
{
    qDeleteAll(mItems);
}

bool ConversationFeedModel::matchesSearch(const QString &searchTerm, const QModelIndex &index) const
{
    if (!index.isValid()) {
        return false;
    }

    ConversationFeedItem *entry = qobject_cast<ConversationFeedItem*>(entryFromIndex(index));
    if (!entry) {
        return false;
    }

    bool foundMatch = false;

    QString value = entry->contactAlias();
    if (value.indexOf(searchTerm, 0, Qt::CaseInsensitive) >= 0) {
        foundMatch = true;
    }

    // Test the phone number
    value = entry->phoneNumber();
    if (ContactModel::instance()->comparePhoneNumbers(value, searchTerm)) {
        foundMatch = true;
    }

    return foundMatch;
}

QString ConversationFeedModel::itemType(const QModelIndex &index) const
{
    return "default";
}

QVariant ConversationFeedModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() > mItems.count()) {
        return QVariant();
    }

    ConversationFeedItem *item = mItems[index.row()];

    switch (role) {
    case ContactId:
        return item->contactId();
    case ContactAlias:
        return item->contactAlias();
    case ContactAvatar:
        return item->contactAvatar();
    case Timestamp:
        return item->timestamp();
    case Date:
        return item->timestamp().date();
    case Incoming:
        return item->incoming();
    case NewItem:
        return item->newItem();
    case ItemType:
        return itemType(index);
    case FeedItem:
        return QVariant::fromValue(const_cast<QObject *>(static_cast<const QObject *>(item)));
    case GroupingProperty:
        if (!item->contactId().isEmpty()) {
            return "contactId";
        } else {
            return "phoneNumber";
        }
    }

    return QVariant();
}

int ConversationFeedModel::rowCount(const QModelIndex &parent) const
{
    // this model is not a tree, so for any valid index, rowCount should be zero
    if (parent.isValid()) {
        return 0;
    }

    return mItems.count();
}

void ConversationFeedModel::addItem(ConversationFeedItem *item)
{
    beginInsertRows(QModelIndex(), mItems.count(), mItems.count());

    connect(item, SIGNAL(contactAliasChanged()), SLOT(onItemChanged()));
    connect(item, SIGNAL(contactAvatarChanged()), SLOT(onItemChanged()));
    connect(item, SIGNAL(contactIdChanged()), SLOT(onItemChanged()));
    connect(item, SIGNAL(incomingChanged()), SLOT(onItemChanged()));
    connect(item, SIGNAL(timestampChanged()), SLOT(onItemChanged()));
    connect(item, SIGNAL(newItemChanged()), SLOT(onItemChanged()));

    mItems.append(item);
    endInsertRows();
}

void ConversationFeedModel::removeItem(ConversationFeedItem *item)
{
    int index = mItems.indexOf(item);
    if (index < 0) {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    mItems.removeAt(index);
    disconnect(item);
    delete item;
    endRemoveRows();
}

void ConversationFeedModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, mItems.count()-1);
    qDeleteAll(mItems);
    mItems.clear();
    endRemoveRows();
}

void ConversationFeedModel::fillContactInfo(ConversationFeedItem *entry, ContactEntry *contact)
{
    entry->setContactId(contact->id().toString());
    entry->setContactAvatar(contact->avatar());
    entry->setContactAlias(contact->displayLabel());
}

void ConversationFeedModel::clearContactInfo(ConversationFeedItem *entry)
{
    entry->setContactAvatar(QUrl(""));
    entry->setContactId("");
    entry->setContactAlias("");
}

void ConversationFeedModel::updateLogForContact(ContactEntry *contactEntry)
{
    // now we need to iterate over the events to look for contacts matching
    int count = mItems.count();
    for (int i = 0; i < count; ++i) {
        ConversationFeedItem *item = mItems[i];
        if (contactMatchesItem(contactEntry, item)) {
            fillContactInfo(item, contactEntry);
            Q_EMIT dataChanged(index(i,0), index(i,0));
        } else if (item->contactId() == contactEntry->idString()) {
            // if the item has the contact id but does not match anymore,
            // clear it.
            clearContactInfo(item);
            Q_EMIT dataChanged(index(i,0), index(i,0));
        }
    }
}

void ConversationFeedModel::onItemChanged()
{
    ConversationFeedItem *item = qobject_cast<ConversationFeedItem*>(sender());
    if (!item) {
        return;
    }

    QModelIndex index = indexFromEntry(item);
    Q_EMIT dataChanged(index, index);
}

QModelIndex ConversationFeedModel::indexFromEntry(ConversationFeedItem *entry) const
{
    int pos = mItems.indexOf(entry);
    if (pos < 0) {
        return QModelIndex();
    }

    return index(pos, 0);
}

ConversationFeedItem *ConversationFeedModel::entryFromIndex(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= mItems.count()) {
        return 0;
    }

    return mItems[index.row()];
}


void ConversationFeedModel::onContactAdded(ContactEntry *contact)
{
    updateLogForContact(contact);
}

void ConversationFeedModel::onContactChanged(ContactEntry *contact)
{
    updateLogForContact(contact);
}

void ConversationFeedModel::onContactRemoved(const QString &contactId)
{
    int count = mItems.count();
    for (int i = 0; i < count; ++i) {
        ConversationFeedItem *item = mItems[i];
        if (item->contactId() == contactId) {
            clearContactInfo(item);
            Q_EMIT dataChanged(index(i,0), index(i,0));
        }
    }
}


bool ConversationFeedModel::contactMatchesItem(ContactEntry *contact, ConversationFeedItem *item) const
{
    // check if any of the contact's phone numbers match in the default implementation
    Q_FOREACH(const QContactPhoneNumber &number, contact->contact().details<QContactPhoneNumber>()) {
        if (ContactModel::comparePhoneNumbers(item->phoneNumber(), number.number())) {
            return true;
        }
    }

    return false;
}

bool ConversationFeedModel::checkNonStandardNumbers(ConversationFeedItem *item)
{
    bool changed = false;

    if (!item) {
        return changed;
    }

    if (item->phoneNumber() == QLatin1String("-2")) {
        item->setContactAlias(QString::fromUtf8(C::dgettext("phone-app", "Private number")));
        item->setPhoneNumber(QLatin1String("-"));
        changed = true;
    } else if (item->phoneNumber() == QLatin1String("-1") || item->phoneNumber() == QLatin1String("#")) {
        item->setContactAlias(QString::fromUtf8(C::dgettext("phone-app", "Unknown number")));
        item->setPhoneNumber(QLatin1String("-"));
        changed = true;
    }

    return changed;
}
