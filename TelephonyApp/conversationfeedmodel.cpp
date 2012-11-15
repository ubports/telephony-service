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

#include "conversationfeedmodel.h"
#include "contactentry.h"

ConversationFeedModel::ConversationFeedModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

bool ConversationFeedModel::matchesSearch(const QString &searchTerm, const QModelIndex &index) const
{
    // TODO: implement
    return true;
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
    case Incoming:
        return item->incoming();
    case NewItem:
        return item->newItem();
    case ItemType:
        return itemType(index);
    case FeedItem:
        return QVariant::fromValue(const_cast<QObject *>(static_cast<const QObject *>(item)));
    case GroupingProperty:
        return "contactId";
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

void ConversationFeedModel::onItemChanged()
{
    ConversationFeedItem *item = qobject_cast<ConversationFeedItem*>(sender());
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


