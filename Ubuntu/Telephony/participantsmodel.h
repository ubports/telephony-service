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

#ifndef PARTICIPANTSMODEL_H
#define PARTICIPANTSMODEL_H

#include "chatentry.h"
#include <QAbstractListModel>
#include <QStringList>
#include <QQmlParserStatus>
#include <QQmlListProperty>

class Participant;

class ParticipantsModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(bool canFetchMore READ canFetchMore NOTIFY canFetchMoreChanged)
    Q_PROPERTY(ChatEntry* chatEntry READ chatEntry WRITE setChatEntry NOTIFY chatEntryChanged)
    Q_ENUMS(Role)

public:
    enum Role {
        IdentifierRole = Qt::UserRole,
        AliasRole,
        RolesRole,
        StateRole
    };

    explicit ParticipantsModel(QObject *parent = 0);
    ~ParticipantsModel();

    Q_INVOKABLE virtual bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE virtual void fetchMore(const QModelIndex &parent = QModelIndex());
    virtual QHash<int, QByteArray> roleNames() const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    Q_INVOKABLE virtual QVariant get(int row) const;

    Q_INVOKABLE void setChatEntry(ChatEntry *entry);
    ChatEntry* chatEntry() const;

    void addParticipantCache(Participant *participant);

    void classBegin();
    void componentComplete();

private Q_SLOTS:
    void addParticipant(Participant *participant);
    void removeParticipant(Participant *participant);

Q_SIGNALS:
    void countChanged();
    void canFetchMoreChanged();
    void chatEntryChanged();

protected:
    bool lessThan(const QString &left, const QString &right) const;
    int positionForItem(const QString &item, bool cache = false) const;

private:
    QHash<int, QByteArray> mRoles;
    QList<Participant*> mParticipants;
    bool mWaitingForQml;
    bool mCanFetchMore;
    ChatEntry *mChatEntry;
    QList<Participant*> mParticipantsCache;
};

#endif // PARTICIPANTSMODEL_H
