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

#include "conversationlogmodel.h"
#include <TelepathyLoggerQt4/Event>
#include <TelepathyLoggerQt4/TextEvent>
#include <TelepathyLoggerQt4/Entity>

QVariant ConversationLogEntry::data(int role) const
{
    switch (role) {
    case ConversationLogModel::Text:
        return text;
    default:
        return LogEntry::data(role);
    }
}

ConversationLogModel::ConversationLogModel(QContactManager *manager, QObject *parent) :
    AbstractLoggerModel(manager, parent)
{
    // set the role names
    QHash<int, QByteArray> roles = roleNames();
    roles[Text] = "text";
    setRoleNames(roles);

    fetchLog(Tpl::EventTypeMaskText);
}

LogEntry *ConversationLogModel::createEntry(const Tpl::EventPtr &event)
{
    ConversationLogEntry *entry = new ConversationLogEntry();
    Tpl::TextEventPtr textEvent = event.dynamicCast<Tpl::TextEvent>();

    if (!textEvent) {
        qWarning() << "The event" << event << "is not a Tpl::TextEvent!";
    }

    entry->text = textEvent->message();
    return entry;
}

void ConversationLogModel::handleDates(const Tpl::EntityPtr &entity, const Tpl::QDateList &dates)
{
    if (!dates.count()) {
        return;
    }
    QDate newestDate = dates.first();

    // search for the newest available date
    Q_FOREACH(const QDate &date, dates) {
        if (date > newestDate) {
            newestDate = date;
        }
    }

    requestEventsForDates(entity, Tpl::QDateList() << newestDate);
}

void ConversationLogModel::handleEvents(const Tpl::EventPtrList &events)
{
    if (!events.count()) {
        return;
    }

    Tpl::EventPtr newestEvent = events.first();

    // search for the newest message
    Q_FOREACH(const Tpl::EventPtr &event, events) {
        if (event->timestamp() > newestEvent->timestamp()) {
            newestEvent = event;
        }
    }

    appendEvents(Tpl::EventPtrList() << newestEvent);
}
