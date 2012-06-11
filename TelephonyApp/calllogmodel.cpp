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

#include "calllogmodel.h"
#include "telepathyhelper.h"
#include <TelepathyLoggerQt4/LogManager>
#include <TelepathyLoggerQt4/PendingDates>
#include <TelepathyLoggerQt4/PendingEntities>
#include <TelepathyLoggerQt4/PendingEvents>
#include <TelepathyLoggerQt4/Entity>
#include <TelepathyLoggerQt4/Event>
#include <TelepathyLoggerQt4/CallEvent>
#include <QContactManager>
#include <QContactDetailFilter>
#include <QContactAvatar>
#include <QContactGuid>
#include <QContactPhoneNumber>

CallLogModel::CallLogModel(QObject *parent) :
    QAbstractListModel(parent)
{
    // set the role names
    QHash<int, QByteArray> roles;
    roles[ContactId] = "contactId";
    roles[ContactAlias] = "contactAlias";
    roles[Avatar] = "avatar";
    roles[PhoneNumber] = "phoneNumber";
    roles[PhoneType] = "phoneType";
    roles[Timestamp] = "timestamp";
    roles[Missed] = "missed";
    roles[Incoming] = "incoming";
    setRoleNames(roles);

    mContactManager = new QContactManager("folks", QMap<QString,QString>(), this);
    connect(mContactManager,
            SIGNAL(contactsAdded(QList<QContactLocalId>)),
            SLOT(onContactsAdded(QList<QContactLocalId>)));
    connect(mContactManager,
            SIGNAL(contactsChanged(QList<QContactLocalId>)),
            SLOT(onContactsChanged(QList<QContactLocalId>)));
    connect(mContactManager,
            SIGNAL(contactsRemoved(QList<QContactLocalId>)),
            SLOT(onContactsRemoved(QList<QContactLocalId>)));
    // FIXME: check if we need to listen to the QContactManager::dataChanged() signal

    fetchCallLog();
}

int CallLogModel::rowCount(const QModelIndex &parent) const
{
    // no child items
    if (parent.isValid()) {
        return 0;
    }

    return mCallEntries.count();
}

QVariant CallLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= mCallEntries.count()) {
        return QVariant();
    }

    switch (role) {
    case ContactId:
        return mCallEntries[index.row()].contactId;
    case ContactAlias:
    case Qt::DisplayRole:
        return mCallEntries[index.row()].contactAlias;
    case Avatar:
    case Qt::DecorationRole:
        return mCallEntries[index.row()].avatar;
    case PhoneNumber:
        return mCallEntries[index.row()].phoneNumber;
    case PhoneType:
        return mCallEntries[index.row()].phoneType;
    case Timestamp:
        return mCallEntries[index.row()].timestamp;
    case Missed:
        return mCallEntries[index.row()].missed;
    case Incoming:
        return mCallEntries[index.row()].incoming;
    default:
        return QVariant();
    }
}

void CallLogModel::fetchCallLog()
{
    Tpl::LogManagerPtr manager = Tpl::LogManager::instance();
    Tpl::PendingEntities *pendingEntities = manager->queryEntities(TelepathyHelper::instance()->account());

    /* Fetching the call log work like this:
       - Start by fetching the entities from the log
       - Once you get the entities, fetch the available dates
       - After you get the dates, fetch the events themselves
     */
    connect(pendingEntities,
            SIGNAL(finished(Tpl::PendingOperation*)),
            SLOT(onPendingEntitiesFinished(Tpl::PendingOperation*)));
}

void CallLogModel::fillContactInfo(CallEntry &entry, const QContact &contact)
{
    QContactGuid guid = contact.detail<QContactGuid>();
    QContactAvatar avatar = contact.detail<QContactAvatar>();
    entry.contactId = guid.guid();
    entry.avatar = avatar.imageUrl();
    entry.localId = contact.localId();
}

void CallLogModel::clearContactInfo(CallEntry &entry)
{
    entry.avatar = "";
    entry.contactId = "";
    entry.localId = QContactLocalId();
}

void CallLogModel::onPendingEntitiesFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingEntities *pe = qobject_cast<Tpl::PendingEntities*>(op);
    if (!pe) {
        return;
    }

    Tpl::EntityPtrList entities = pe->entities();
    Tpl::LogManagerPtr manager = Tpl::LogManager::instance();
    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    foreach(Tpl::EntityPtr entity, entities) {
        Tpl::PendingDates *pendingDates = manager->queryDates(account, entity, Tpl::EventTypeMaskAny);

        connect(pendingDates,
                SIGNAL(finished(Tpl::PendingOperation*)),
                SLOT(onPendingDatesFinished(Tpl::PendingOperation*)));
    }
}

void CallLogModel::onPendingDatesFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingDates *pd = qobject_cast<Tpl::PendingDates*>(op);
    if (!pd) {
        return;
    }

    Tpl::QDateList dates = pd->dates();
    Tpl::LogManagerPtr manager = Tpl::LogManager::instance();
    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    foreach(QDate date, dates) {
        Tpl::PendingEvents *pendingEvents = manager->queryEvents(account, pd->entity(), Tpl::EventTypeMaskAny, date);
        connect(pendingEvents,
                SIGNAL(finished(Tpl::PendingOperation*)),
                SLOT(onPendingEventsFinished(Tpl::PendingOperation*)));
    }
}

void CallLogModel::onPendingEventsFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingEvents *pe = qobject_cast<Tpl::PendingEvents*>(op);
    if (!pe) {
        return;
    }

    Tpl::EventPtrList events = pe->events();

    // add the events to the list
    foreach(Tpl::EventPtr event, events) {
        Tpl::CallEventPtr callEvent = event.dynamicCast<Tpl::CallEvent>();
        if (callEvent) {
            CallEntry entry;
            entry.incoming = (callEvent->receiver()->entityType() == Tpl::EntityTypeSelf);
            entry.timestamp = callEvent->timestamp();
            entry.missed = (callEvent->endReason() == Tp::CallStateChangeReasonNoAnswer);

            Tpl::EntityPtr remoteEntity = entry.incoming ? callEvent->sender() : callEvent->receiver();
            entry.contactAlias = remoteEntity->alias();
            entry.phoneNumber = remoteEntity->identifier();

            // fetch the QContact object
            QContactDetailFilter filter;
            filter.setDetailDefinitionName(QContactPhoneNumber::DefinitionName, QContactPhoneNumber::FieldNumber);
            filter.setValue(remoteEntity->identifier());
            filter.setMatchFlags(QContactFilter::MatchPhoneNumber);

            QList<QContact> contacts = mContactManager->contacts(filter);
            if (contacts.count() > 0) {
                // if more than one contact matches, use the first one
                fillContactInfo(entry, contacts[0]);
            }

            beginInsertRows(QModelIndex(), mCallEntries.count(), mCallEntries.count());
            mCallEntries.append(entry);
            endInsertRows();
        }
    }
}

void CallLogModel::onContactsAdded(const QList<QContactLocalId> &contactIds)
{
    QList<QContact> contacts = mContactManager->contacts(contactIds);

    // now we need to iterate over the events to look for contacts matching
    int count = mCallEntries.count();
    for (int i = 0; i < count; ++i) {
        QString phoneNumber = mCallEntries[i].phoneNumber;

        // and for each event, we check if it matches
        foreach (const QContact &contact, contacts) {
            bool foundMatch = false;
            QList<QContactPhoneNumber> phoneNumbers = contact.details<QContactPhoneNumber>();
            foreach (const QContactPhoneNumber &number, phoneNumbers) {
                if (phoneNumber == number.number()) {
                    fillContactInfo(mCallEntries[i], contact);
                    emit dataChanged(index(i,0), index(i,0));
                    foundMatch = true;
                    break;
                }
            }
            if (foundMatch) {
                break;
            }
        }
    }
}

void CallLogModel::onContactsChanged(const QList<QContactLocalId> &contactIds)
{
    int count = mCallEntries.count();
    for (int i = 0; i < count; ++i) {
        int position = contactIds.indexOf(mCallEntries[i].localId);
        if (position >= 0) {
            fillContactInfo(mCallEntries[i], mContactManager->contact(contactIds[position]));
            emit dataChanged(index(i, 0), index(i, 0));
        }
    }
}

void CallLogModel::onContactsRemoved(const QList<QContactLocalId> &contactIds)
{
    int count = mCallEntries.count();
    for (int i = 0; i < count; ++i) {
        CallEntry &entry = mCallEntries[i];
        if (contactIds.indexOf(entry.localId) >=0) {
            clearContactInfo(entry);
            emit dataChanged(index(i,0), index(i,0));
        }
    }
}
