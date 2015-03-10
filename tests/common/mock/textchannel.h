/**
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *          Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 */

#ifndef MOCKTEXTCHANNEL_H
#define MOCKTEXTCHANNEL_H

#include <QObject>

#include <TelepathyQt/Constants>
#include <TelepathyQt/BaseChannel>
#include <TelepathyQt/Types>
#include <TelepathyQt/DBusError>

#include "connection.h"

class MockConnection;

class MockTextChannel : public QObject
{
    Q_OBJECT
public:
    MockTextChannel(MockConnection *conn, QStringList recipients, uint targetHandle, QObject *parent = 0);
    QString sendMessage(const Tp::MessagePartList& message, uint flags, Tp::DBusError* error);
    void messageReceived(const QString & message, const QVariantMap &info);
    Tp::BaseChannelPtr baseChannel();
    void messageAcknowledged(const QString &id);
    void mmsReceived(const QString &id, const QVariantMap &properties);

    void addMembers(QStringList recipients);
    QStringList recipients() const;
    Tp::UIntList members();
    void onAddMembers(const Tp::UIntList& handles, const QString& message, Tp::DBusError* error);
    void onRemoveMembers(const Tp::UIntList& handles, const QString& message, Tp::DBusError* error);

public Q_SLOTS:
    void placeDeliveryReport(const QString &messageId, const QString &status);

Q_SIGNALS:
    void messageRead(const QString &id);
    void messageSent(const QString &message, const QVariantMap &info);

private:
    ~MockTextChannel();
    Tp::BaseChannelPtr mBaseChannel;
    QStringList mRecipients;
    MockConnection *mConnection;
    uint mTargetHandle;
    Tp::BaseChannelMessagesInterfacePtr mMessagesIface;
    Tp::BaseChannelGroupInterfacePtr mGroupIface;
    Tp::BaseChannelTextTypePtr mTextChannel;
    uint mMessageCounter;
    Tp::UIntList mMembers;
};

#endif // MOCKTEXTCHANNEL_H
