/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#ifndef CALLAGENT_H
#define CALLAGENT_H

#include <QObject>
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/Farstream/Channel>
#include "farstreamchannel.h"

class CallAgent : public QObject
{
    Q_OBJECT
public:
    explicit CallAgent(const Tp::CallChannelPtr &channel, QObject *parent = 0);
    ~CallAgent();

protected Q_SLOTS:
    void onCallChannelInvalidated();
    void onCallStateChanged(Tp::CallState state);
    void onContentAdded(const Tp::CallContentPtr &content);
    void onStreamAdded(const Tp::CallStreamPtr &stream);

    void onFarstreamChannelCreated(Tp::PendingOperation *op);
    void onFarstreamChannelStateChanged();

private:
    Tp::CallChannelPtr mChannel;
    FarstreamChannel *mFarstreamChannel;
};

#endif // CALLAGENT_H
