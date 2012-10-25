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

#ifndef CONVERSATIONFEEDMODEL_H
#define CONVERSATIONFEEDMODEL_H

#include <QAbstractListModel>
#include "conversationfeeditem.h"

class ConversationFeedModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ConversationFeedModel(QObject *parent = 0);

    enum FeedRoles {
        ContactId = Qt::UserRole,
        ContactAlias,
        ContactAvatar,
        Timestamp,
        Incoming,
        ItemType,
        FeedItem
    };

    /**
     * The strategy used to define how this feed's items are going to be displayed by
     * the aggregator.
     */
    enum DisplayStrategy {
        ShowAllEvents, ///< Show all events from the feed
        ShowLatestEvents, ///< Show only the latest event for each grouping key
        ShowGroupedEvents ///< Group events that happened in sequence on time
    };

    /**
     * Returns the display strategy that should be used by the feed.
     *
     * \sa DisplayStrategy
     * @return
     */
    virtual DisplayStrategy displayStrategy() const;

    /**
     * Returns the key that should be used when grouping or showing only the
     * latest events for a given id.
     * @param index A valid index of the model.
     * @return the key to use for grouping this index
     */
    virtual QString groupingKeyForIndex(const QModelIndex &index) const;

    /**
     * Reports if a given index matches the search term
     * @param searchTerm The search term
     * @param index A valid index of the model.
     * @return true if the item matches the search term
     */
    virtual bool matchesSearch(const QString &searchTerm, const QModelIndex &index) const;

    /**
     * Returns a string identifying the type of a given item
     * @param index A valid index
     * @return The item type
     */
    virtual QString itemType(const QModelIndex &index) const;

    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent) const;

protected:
    void addItem(ConversationFeedItem *item);
    void removeItem(ConversationFeedItem *item);
    void clear();
    QList<ConversationFeedItem*> mItems;
};

#endif // CONVERSATIONFEEDMODEL_H
