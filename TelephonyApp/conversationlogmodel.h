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

#ifndef CONVERSATIONLOGMODEL_H
#define CONVERSATIONLOGMODEL_H

#include "abstractloggermodel.h"

class ConversationLogEntry : public LogEntry {
public:
    QVariant data(int role) const;
    QString text;
};

class ConversationLogModel : public AbstractLoggerModel
{
    Q_OBJECT
public:
    enum ConversationLogRoles {
        Text = AbstractLoggerModel::LastLogRole
    };

    explicit ConversationLogModel(QContactManager *manager, QObject *parent = 0);
    
protected:
    LogEntry *createEntry(const Tpl::EventPtr &event);
    void handleDates(const Tpl::EntityPtr &entity, const Tpl::QDateList &dates);
    void handleEvents(const Tpl::EventPtrList &events);
};

#endif // CONVERSATIONLOGMODEL_H
