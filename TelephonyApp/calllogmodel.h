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

#ifndef CALLLOGMODEL_H
#define CALLLOGMODEL_H

#include "abstractloggermodel.h"
#include <QTime>
#include <TelepathyQt/CallChannel>

class CallLogEntry : public LogEntry {
public:
    QVariant data(int role) const;
    QTime duration;
    bool missed;
};

class CallLogModel : public AbstractLoggerModel
{
    Q_OBJECT
public:
    enum CallLogRoles {
        Duration = AbstractLoggerModel::LastLogRole,
        Missed
    };

    explicit CallLogModel(QObject *parent = 0);

public Q_SLOTS:
    void onCallEnded(const Tp::CallChannelPtr &channel);
    
protected:
    LogEntry *createEntry(const Tpl::EventPtr &event);
};

#endif // CALLLOGMODEL_H
