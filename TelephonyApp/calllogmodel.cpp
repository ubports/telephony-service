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
#include <TelepathyLoggerQt4/Event>
#include <TelepathyLoggerQt4/CallEvent>

QVariant CallLogEntry::data(int role) const
{
    switch(role) {
    case CallLogModel::Duration:
        return duration;
    case CallLogModel::Missed:
        return missed;
    default:
        return LogEntry::data(role);
    }
}

CallLogModel::CallLogModel(QObject *parent) :
    AbstractLoggerModel(parent)
{
    // set the role names
    QHash<int, QByteArray> roles = roleNames();
    roles[Duration] = "duration";
    roles[Missed] = "missed";
    setRoleNames(roles);

    fetchLog(Tpl::EventTypeMaskCall);
}

LogEntry *CallLogModel::createEntry(const Tpl::EventPtr &event)
{
    CallLogEntry *entry = new CallLogEntry();
    Tpl::CallEventPtr callEvent = event.dynamicCast<Tpl::CallEvent>();

    if (callEvent.isNull()) {
        qWarning() << "The event" << event << "is not a Tpl::CallEvent!";
    }

    entry->missed = (callEvent->endReason() == Tp::CallStateChangeReasonNoAnswer);
    entry->duration = callEvent->duration();
    return entry;
}
