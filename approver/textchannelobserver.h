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

#ifndef TEXTCHANNELOBSERVER_H
#define TEXTCHANNELOBSERVER_H

#include <QObject>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>

class TextChannelObserver : public QObject
{
    Q_OBJECT
public:
    explicit TextChannelObserver(QObject *parent = 0);

public Q_SLOTS:
    void onTextChannelAvailable(Tp::TextChannelPtr textChannel);

protected:
    void showNotificationForMessage(const Tp::ReceivedMessage &message);
    Tp::TextChannelPtr channelFromPath(const QString &path);

protected Q_SLOTS:
    void onTextChannelInvalidated();
    void onMessageReceived(const Tp::ReceivedMessage &message);
    void onPendingMessageRemoved(const Tp::ReceivedMessage &message);

private:
    QList<Tp::TextChannelPtr> mChannels;
};

#endif // TEXTCHANNELOBSERVER_H
