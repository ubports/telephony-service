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

class CallLogEntry : public LoggerItem {
    Q_OBJECT
    Q_PROPERTY(QTime duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(bool missed READ missed WRITE setMissed NOTIFY missedChanged)
public:
    explicit CallLogEntry(QObject *parent = 0) : LoggerItem(parent) { }
    void setDuration(const QTime &duration) { mDuration = duration; Q_EMIT durationChanged(); }
    QTime duration() { return mDuration; }
    void setMissed(bool missed) { mMissed = missed; Q_EMIT missedChanged(); }
    bool missed() { return mMissed; }

Q_SIGNALS:
    void durationChanged();
    void missedChanged();

private:
    QTime mDuration;
    bool mMissed;
};

class CallLogModel : public AbstractLoggerModel
{
    Q_OBJECT
public:
    explicit CallLogModel(QObject *parent = 0);

    QString itemType(const QModelIndex &index) const;

    bool matchesSearch(const QString &searchTerm, const QModelIndex &index) const;


public Q_SLOTS:
    void populate();
    void onCallEnded(const Tp::CallChannelPtr &channel);

protected:
    LoggerItem *createEntry(const Tpl::EventPtr &event);
};

#endif // CALLLOGMODEL_H
