/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#ifndef CALLHANDLER_H
#define CALLHANDLER_H

#include <QtCore/QMap>
#include <QDBusInterface>
#include <TelepathyQt/CallChannel>

class TelepathyHelper;

class CallHandler : public QObject
{
    Q_OBJECT

public:
    explicit CallHandler(QObject *parent = 0);
    
    void startCall(const QString &phoneNumber);

public Q_SLOTS:
    void onCallChannelAvailable(Tp::CallChannelPtr channel);
    void onContactsAvailable(Tp::PendingOperation *op);

private:
    void refreshProperties();

    QMap<QString, Tp::ContactPtr> mContacts;
    QString mVoicemailNumber;
    TelepathyHelper *mTelepathyHelper;
    QList<Tp::CallChannelPtr> mCallChannels;
};

#endif // CALLHANDLER_H
