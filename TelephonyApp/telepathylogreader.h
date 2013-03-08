/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TELEPATHYLOGREADER_H
#define TELEPATHYLOGREADER_H

#include <QObject>
#include <TelepathyLoggerQt/PendingOperation>
#include <TelepathyLoggerQt/Types>
#include <TelepathyLoggerQt/LogManager>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QUrl>

class TelepathyLogReader : public QObject
{
    Q_OBJECT
public:
    static TelepathyLogReader *instance();

public Q_SLOTS:
    void fetchLog();

protected:
    void requestDatesForEntities(const Tpl::EntityPtrList &entities);
    void requestEventsForDates(const Tpl::EntityPtr &entity, const Tpl::QDateList &dates);

Q_SIGNALS:
    void loadedCallEvent(const QString &phoneNumber, bool incoming, const QDateTime &timestamp, const QTime &duration, bool missed, bool newEvent);

    // the order of parameters here is this way to match the ones already in use in ChatManager and MessageLogModel
    void loadedMessageEvent(const QString &phoneNumber, const QString &message, bool incoming, const QDateTime &timestamp, const QString &messageId, bool newEvent);

protected Q_SLOTS:
    void onPendingEntitiesFinished(Tpl::PendingOperation *op);
    void onPendingDatesFinished(Tpl::PendingOperation *op);
    void onPendingEventsFinished(Tpl::PendingOperation *op);

protected:
    Tpl::LogManagerPtr mLogManager;

private:
    explicit TelepathyLogReader(QObject *parent = 0);

};

#endif // CALLLOGMODEL_H
