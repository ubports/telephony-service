/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CALLCHANNELOBSERVER_H
#define CALLCHANNELOBSERVER_H

#include <QObject>
#include <TelepathyQt/CallChannel>

class CallChannelObserver : public QObject
{
    Q_OBJECT
public:
    explicit CallChannelObserver(QObject *parent = 0);

public Q_SLOTS:
    void onCallChannelAvailable(Tp::CallChannelPtr callChannel);

Q_SIGNALS:
    void callEnded(Tp::CallChannelPtr callChannel);

protected Q_SLOTS:
    void onCallStateChanged(Tp::CallState state);
    void onHoldChanged();

private:
    QList<Tp::CallChannelPtr> mChannels;
};

#endif // CALLCHANNELOBSERVER_H
