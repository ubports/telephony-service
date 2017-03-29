/*
 * Copyright (C) 2013-2017 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "participantsmodel.h"
#include <participant.h>
#include <QDebug>

Q_DECLARE_METATYPE(Participant)

ParticipantsModel::ParticipantsModel(QObject *parent) :
    QAbstractListModel(parent), mWaitingForQml(false), mCanFetchMore(true), mChatEntry(NULL)
{
    qRegisterMetaType<Participant>();
    mRoles[AliasRole] = "alias";
    mRoles[IdentifierRole] = "identifier";
    mRoles[RolesRole] = "roles";
    mRoles[StateRole] = "state";

    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()), this, SIGNAL(countChanged()));
}

bool ParticipantsModel::canFetchMore(const QModelIndex &parent) const
{
    return !mParticipantsCache.isEmpty();
}

ParticipantsModel::~ParticipantsModel()
{
}

void ParticipantsModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid() ) {
        return;
    }

    int max = 14;
    while (max >= 0 && !mParticipantsCache.isEmpty()) {
        addParticipant(mParticipantsCache.takeFirst());
        max--;
    }

    if (mParticipantsCache.isEmpty()) {
        mCanFetchMore = false;
        Q_EMIT canFetchMoreChanged();
    }
}

int ParticipantsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return mParticipants.count();
}

QHash<int, QByteArray> ParticipantsModel::roleNames() const
{
    return mRoles;
}

void ParticipantsModel::addParticipantCache(Participant *participant)
{
    int pos = positionForItem(participant->identifier(), true);
    mParticipantsCache.insert(pos, participant);
}

void ParticipantsModel::addParticipant(Participant *participant)
{
    int pos = positionForItem(participant->identifier());
    beginInsertRows(QModelIndex(), pos, pos);
    mParticipants.insert(pos, participant);
    endInsertRows();
}

void ParticipantsModel::removeParticipant(Participant *participant)
{
    int pos = mParticipants.indexOf(participant);
    if (pos >= 0) {
        beginRemoveRows(QModelIndex(), pos, pos);
        mParticipants.removeAt(pos);
        endRemoveRows();
    }
    pos = mParticipantsCache.indexOf(participant);
    if (pos >= 0) {
        mParticipantsCache.removeAt(pos);
    }
}

QVariant ParticipantsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) {
        return QVariant();
    }
    switch (role) {
    case IdentifierRole:
        return mParticipants[index.row()]->identifier();
    break;
    case AliasRole:
        return mParticipants[index.row()]->alias();
    break;
    case StateRole:
        return 0;
    break;
    case RolesRole:
        return mParticipants[index.row()]->roles();
    break;
    }
    return QVariant();
}

bool ParticipantsModel::lessThan(const QString &left, const QString &right) const
{
    // this method will push participant with names starting with non-letter
    // characters to the end of the list
    if (left.isEmpty() || right.isEmpty()) {
        return false;
    }
    if (left.at(0).isLetter() && right.at(0).isLetter()) {
        return left.localeAwareCompare(right) < 0;
    }
    if (!left.at(0).isLetter() && right.at(0).isLetter()) {
        return false;
    }
    if (left.at(0).isLetter() && !right.at(0).isLetter()) {
        return true;
    }

    return false;
}

int ParticipantsModel::positionForItem(const QString &item, bool cache) const
{
    // do a binary search for the item position on the list
    int lowerBound = 0;
    int upperBound = cache ? mParticipantsCache.count() - 1 : rowCount() - 1;
    if (upperBound < 0) {
        return 0;
    }

    while (true) {
        int pos = (upperBound + lowerBound) / 2;
        const QString posItem = cache ? mParticipantsCache[pos]->identifier() : index(pos).data(IdentifierRole).toString();
        if (lowerBound == pos) {
            if (lessThan(item, posItem)) {
                return pos;
            }
        }
        if (lessThan(posItem, item)) {
            lowerBound = pos + 1;          // its in the upper
            if (lowerBound > upperBound) {
                return pos += 1;
            }
        } else if (lowerBound > upperBound) {
            return pos;
        } else {
            upperBound = pos - 1;          // its in the lower
        }
    }
}

void ParticipantsModel::classBegin()
{
    mWaitingForQml = true;
}

void ParticipantsModel::componentComplete()
{
    mWaitingForQml = false;
}

QVariant ParticipantsModel::get(int row) const
{
    QVariantMap data;
    QModelIndex idx = index(row, 0);
    if (idx.isValid()) {
        QHash<int, QByteArray> roles = roleNames();
        Q_FOREACH(int role, roles.keys()) {
            data.insert(roles[role], idx.data(role));
        }
    }

    return data;
}

ChatEntry* ParticipantsModel::chatEntry() const
{
    return mChatEntry;
}

void ParticipantsModel::setChatEntry(ChatEntry *entry)
{
    if (mChatEntry == entry) {
        return;
    }
    ChatEntry *previousChatEntry = mChatEntry;
    mChatEntry = entry;
    if (!entry) {
        return;
    }
    if (previousChatEntry) {
        previousChatEntry->disconnect(this);
    }
    connect(mChatEntry, SIGNAL(participantAdded(Participant *)), SLOT(addParticipant(Participant *)));
    connect(mChatEntry, SIGNAL(participantRemoved(Participant *)), SLOT(removeParticipant(Participant *)));
    Q_FOREACH(Participant *participant, mChatEntry->allParticipants()) {
        addParticipantCache(participant);
    }
    fetchMore();
    mCanFetchMore = !mParticipantsCache.isEmpty();
    Q_EMIT canFetchMoreChanged();
    Q_EMIT chatEntryChanged();
}
