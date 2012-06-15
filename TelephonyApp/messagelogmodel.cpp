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

#include "messagelogmodel.h"
#include <TelepathyLoggerQt4/Event>
#include <TelepathyLoggerQt4/TextEvent>
#include <TelepathyLoggerQt4/Entity>

QVariant MessageLogEntry::data(int role) const
{
    switch (role) {
    case MessageLogModel::Message:
        return message;
    case MessageLogModel::Date:
        return timestamp.date().toString(Qt::DefaultLocaleLongDate);
    default:
        return LogEntry::data(role);
    }
}

MessageLogModel::MessageLogModel(QContactManager *manager, QObject *parent) :
    AbstractLoggerModel(manager, parent)
{
    // set the role names
    QHash<int, QByteArray> roles = roleNames();
    roles[Message] = "message";
    roles[Date] = "date";
    setRoleNames(roles);
}


QString MessageLogModel::phoneNumber() const
{
    return mPhoneNumber;
}

void MessageLogModel::setPhoneNumber(QString value)
{
    if (mPhoneNumber != value) {
        clear();
        mPhoneNumber = value;

        if (!mPhoneNumber.isEmpty()) {
            fetchLog(Tpl::EventTypeMaskText);
        }
    }
}

LogEntry *MessageLogModel::createEntry(const Tpl::EventPtr &event)
{
    MessageLogEntry *entry = new MessageLogEntry();
    Tpl::TextEventPtr textEvent = event.dynamicCast<Tpl::TextEvent>();

    if (!textEvent) {
        qWarning() << "The event" << event << "is not a Tpl::TextEvent!";
    }

    entry->message = textEvent->message();
    return entry;
}

void MessageLogModel::handleEntities(const Tpl::EntityPtrList &entities)
{
    // search for the entity that matches the phone number for this conversation
    // FIXME: we probably need a more reliable way than string matching for comparing phone numbers
    Q_FOREACH(const Tpl::EntityPtr &entity, entities) {
        if (entity->identifier() == mPhoneNumber) {
            requestDatesForEntities(Tpl::EntityPtrList() << entity);
            return;
        }
    }
}

