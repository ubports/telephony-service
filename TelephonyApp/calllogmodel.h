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

#ifndef CALLLOGMODEL_H
#define CALLLOGMODEL_H

#include "conversationfeedmodel.h"
#include "conversationfeeditem.h"
#include <QTime>
#include <TelepathyQt/CallChannel>

class CallLogEntry : public ConversationFeedItem {
    Q_OBJECT
    Q_PROPERTY(QTime duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(bool missed READ missed WRITE setMissed NOTIFY missedChanged)
public:
    explicit CallLogEntry(QObject *parent = 0) : ConversationFeedItem(parent) { }
    void setDuration(const QTime &duration) { mDuration = duration; Q_EMIT durationChanged(); }
    QTime duration() { return mDuration; }
    void setMissed(bool missed) { mMissed = missed; Q_EMIT missedChanged(); }
    bool missed() { return mMissed; }

public Q_SLOTS:

Q_SIGNALS:
    void durationChanged();
    void missedChanged();

private:
    QTime mDuration;
    bool mMissed;
};

class CallLogModel : public ConversationFeedModel
{
    Q_OBJECT
public:
    explicit CallLogModel(QObject *parent = 0);

    QString itemType(const QModelIndex &index) const;

public Q_SLOTS:
    void addCallEvent(const QString &phoneNumber, bool incoming, const QDateTime &timestamp, const QTime &duration, bool missed, bool newEvent);
    void onCallEnded(const Tp::CallChannelPtr &channel);

};

#endif // CALLLOGMODEL_H
