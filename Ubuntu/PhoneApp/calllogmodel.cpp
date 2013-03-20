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

#include "calllogmodel.h"
#include "contactmodel.h"
#include "telepathyhelper.h"
#include <TelepathyQt/Contact>

CallLogModel::CallLogModel(QObject *parent) :
    ConversationFeedModel(parent)
{
}

void CallLogModel::onCallEnded(const Tp::CallChannelPtr &channel)
{
    Tp::Contacts contacts = channel->remoteMembers();
    if (contacts.isEmpty()) {
        qWarning() << "Call channel had no remote contacts:" << channel;
        return;
    }

    QString phoneNumber;
    // FIXME: handle conference call
    Q_FOREACH(const Tp::ContactPtr &contact, contacts) {
        phoneNumber = contact->id();
        break;
    }

    // fill the call info
    QDateTime timestamp = channel->property("timestamp").toDateTime();
    bool incoming = channel->initiatorContact() != TelepathyHelper::instance()->account()->connection()->selfContact();
    QTime duration(0, 0, 0);
    bool missed = incoming && channel->callStateReason().reason == Tp::CallStateChangeReasonNoAnswer;

    if (!missed) {
        QDateTime activeTime = channel->property("activeTimestamp").toDateTime();
        duration = duration.addSecs(activeTime.secsTo(QDateTime::currentDateTime()));
    }

    // and finally add the entry
    addCallEvent(phoneNumber, incoming, timestamp, duration, missed, true);
}

QString CallLogModel::itemType(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return "call";
}

void CallLogModel::addCallEvent(const QString &phoneNumber, bool incoming, const QDateTime &timestamp, const QTime &duration, bool missed, bool newEvent)
{
    CallLogEntry *entry = new CallLogEntry(this);
    entry->setPhoneNumber(phoneNumber);
    entry->setContactAlias(phoneNumber);
    entry->setIncoming(incoming);
    entry->setTimestamp(timestamp);
    entry->setDuration(duration);
    entry->setMissed(missed);
    entry->setNewItem(newEvent);

    // try to fill the contact info
    ContactEntry *contact = ContactModel::instance()->contactFromPhoneNumber(phoneNumber);
    if (contact) {
        fillContactInfo(entry, contact);
    } else {
        checkNonStandardNumbers(entry);
    }

    addItem(entry);
}
