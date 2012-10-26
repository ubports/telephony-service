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
#include "contactmodel.h"
#include "telepathyhelper.h"
#include <TelepathyLoggerQt/Entity>
#include <TelepathyLoggerQt/Event>
#include <TelepathyLoggerQt/CallEvent>
#include <TelepathyQt/Contact>

CallLogModel::CallLogModel(QObject *parent) :
    AbstractLoggerModel(parent)
{
}

void CallLogModel::populate()
{
    fetchLog(Tpl::EventTypeMaskCall, EntityTypeList() << Tpl::EntityTypeContact
                                                      << Tpl::EntityTypeSelf);
}

void CallLogModel::onCallEnded(const Tp::CallChannelPtr &channel)
{
    Tp::Contacts contacts = channel->remoteMembers();
    if (contacts.isEmpty()) {
        qWarning() << "Call channel had no remote contacts:" << channel;
        return;
    }

    CallLogEntry *entry = new CallLogEntry();
    // FIXME: handle conference call
    Q_FOREACH(const Tp::ContactPtr &contact, contacts) {
        entry->setPhoneNumber(contact->id());
        break;
    }

    // fill the contact info
    ContactEntry *contact = ContactModel::instance()->contactFromPhoneNumber(entry->phoneNumber());
    entry->setContactAlias(entry->phoneNumber());
    if (contact) {
        fillContactInfo(entry, contact);
    } else {
        checkNonStandardNumbers(entry);
    }

    // fill the call info
    entry->setTimestamp(channel->property("timestamp").toDateTime());
    bool isIncoming = channel->initiatorContact() != TelepathyHelper::instance()->account()->connection()->selfContact();
    entry->setIncoming(isIncoming);
    entry->setDuration(QTime(0,0,0));

    // outgoing calls can be missed calls?
    if (entry->incoming() && channel->callStateReason().reason == Tp::CallStateChangeReasonNoAnswer) {
        entry->setMissed(true);
    } else {
        QDateTime activeTime = channel->property("activeTime").toDateTime();
        entry->setDuration(entry->duration().addSecs(activeTime.secsTo(QDateTime::currentDateTime())));
        entry->setMissed(false);
    }

    // and finally add the entry
    appendEntry(entry);
}

LoggerItem *CallLogModel::createEntry(const Tpl::EventPtr &event)
{
    CallLogEntry *entry = new CallLogEntry();
    Tpl::CallEventPtr callEvent = event.dynamicCast<Tpl::CallEvent>();

    if (callEvent.isNull()) {
        qWarning() << "The event" << event << "is not a Tpl::CallEvent!";
    }

    entry->setMissed(callEvent->endReason() == Tp::CallStateChangeReasonNoAnswer);
    entry->setDuration(callEvent->duration());
    return entry;
}

bool CallLogModel::matchesSearch(const QString &searchTerm, const QModelIndex &index) const
{
    CallLogEntry *entry = dynamic_cast<CallLogEntry*>(index.data(ConversationFeedModel::FeedItem).value<CallLogEntry*>());
    if (!entry) {
        return false;
    }

    QString value = entry->contactAlias();
    if (value.indexOf(searchTerm, 0, Qt::CaseInsensitive) >= 0) {
        // if onlyLatest option is set, we just return one contact alias match
        return true;
    }

    // Test the phone number
    value = entry->phoneNumber();
    if (ContactModel::instance()->comparePhoneNumbers(value, searchTerm)) {
        // if onlyLatest option is set, we just return one contact alias match
        return true;
    }

    return false;
}

QString CallLogModel::itemType(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return "call";
}
