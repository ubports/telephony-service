/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#ifndef CALLMANAGER_H
#define CALLMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/ReceivedMessage>

class CallManager : public QObject
{
    Q_OBJECT
public:
    explicit CallManager(QObject *parent = 0);
    
    Q_INVOKABLE bool isChattingToContact(const QString &contactId);
    Q_INVOKABLE void startCall(const QString &contactId);
    Q_INVOKABLE void endCall(const QString &contactId);

signals:
    void callReady(const QString &contactId);

public Q_SLOTS:
    void onCallChannelAvailable(Tp::CallChannelPtr channel);
    void onContactsAvailable(Tp::PendingOperation *op);


private:
    QMap<QString, Tp::CallChannelPtr> mChannels;
    QMap<QString, Tp::ContactPtr> mContacts;
};

#endif // CALLMANAGER_H
