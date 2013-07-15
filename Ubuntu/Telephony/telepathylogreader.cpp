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

#include "telepathylogreader.h"
#include "telepathyhelper.h"
#include "contactentry.h"
#include "contactmodel.h"
#include "conversationfeeditem.h"
#include <TelepathyLoggerQt/LogManager>
#include <TelepathyLoggerQt/PendingDates>
#include <TelepathyLoggerQt/PendingEntities>
#include <TelepathyLoggerQt/PendingEvents>
#include <TelepathyLoggerQt/Entity>
#include <TelepathyLoggerQt/Event>
#include <TelepathyLoggerQt/CallEvent>
#include <TelepathyLoggerQt/TextEvent>
#include <QContact>
#include <QContactPhoneNumber>

TelepathyLogReader::TelepathyLogReader(QObject *parent) :
    QObject(parent), mLogManager(Tpl::LogManager::instance())
{
}

TelepathyLogReader *TelepathyLogReader::instance()
{
    static TelepathyLogReader *self = new TelepathyLogReader();
    return self;
}

void TelepathyLogReader::fetchLog()
{
    Tpl::PendingEntities *pendingEntities = mLogManager->queryEntities(TelepathyHelper::instance()->account());

    /* Fetching the log work like this:
       - Start by fetching the entities from the log
       - Once you get the entities, fetch the available dates
       - After you get the dates, fetch the events themselves
     */

    connect(pendingEntities,
            SIGNAL(finished(Tpl::PendingOperation*)),
            SLOT(onPendingEntitiesFinished(Tpl::PendingOperation*)));
}

void TelepathyLogReader::requestDatesForEntities(const Tpl::EntityPtrList &entities)
{
    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    Q_FOREACH(Tpl::EntityPtr entity, entities) {
        Tpl::PendingDates *pendingDates = mLogManager->queryDates(account, entity, Tpl::EventTypeMaskAny);

        connect(pendingDates,
                SIGNAL(finished(Tpl::PendingOperation*)),
                SLOT(onPendingDatesFinished(Tpl::PendingOperation*)));
    }
}

void TelepathyLogReader::requestEventsForDates(const Tpl::EntityPtr &entity, const Tpl::QDateList &dates)
{
    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    Q_FOREACH(QDate date, dates) {
        Tpl::PendingEvents *pendingEvents = mLogManager->queryEvents(account, entity, Tpl::EventTypeMaskAny, date);
        connect(pendingEvents,
                SIGNAL(finished(Tpl::PendingOperation*)),
                SLOT(onPendingEventsFinished(Tpl::PendingOperation*)));
    }
}

void TelepathyLogReader::onPendingEntitiesFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingEntities *pe = qobject_cast<Tpl::PendingEntities*>(op);
    if (!pe) {
        return;
    }

    // request the dates for all the entities
    requestDatesForEntities(pe->entities());
}

void TelepathyLogReader::onPendingDatesFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingDates *pd = qobject_cast<Tpl::PendingDates*>(op);
    if (!pd) {
        return;
    }

    // request all events
    requestEventsForDates(pd->entity(), pd->dates());
}

void TelepathyLogReader::onPendingEventsFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingEvents *pe = qobject_cast<Tpl::PendingEvents*>(op);
    if (!pe) {
        return;
    }

    Q_FOREACH(const Tpl::EventPtr &event, pe->events()) {
        bool incoming = event->receiver()->entityType() == Tpl::EntityTypeSelf;
        Tpl::EntityPtr remoteEntity = incoming ? event->sender() : event->receiver();

        QString phoneNumber = remoteEntity->identifier();
        QDateTime timestamp = event->timestamp();

        Tpl::CallEventPtr callEvent = event.dynamicCast<Tpl::CallEvent>();
        Tpl::TextEventPtr textEvent = event.dynamicCast<Tpl::TextEvent>();

        if (!callEvent.isNull()) {
            bool missed = incoming && callEvent->endReason() == Tp::CallStateChangeReasonNoAnswer;
            QTime duration = callEvent->duration();

            Q_EMIT loadedCallEvent(phoneNumber, incoming, timestamp, duration, missed, false);
        }

        if (!textEvent.isNull()) {
            QString message = textEvent->message();
            QString messageId = textEvent->messageToken();

            Q_EMIT loadedMessageEvent(phoneNumber, message, incoming, timestamp, messageId, false);
        }
    }
}
