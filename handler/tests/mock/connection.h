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

#ifndef MOCKCONNECTION_H
#define MOCKCONNECTION_H

// qt
#include <QTimer>

// telepathy-qt
#include <TelepathyQt/BaseConnection>
#include <TelepathyQt/BaseChannel>
#include <TelepathyQt/BaseCall>
#include <TelepathyQt/AbstractAdaptor>
#include <TelepathyQt/DBusError>

// mock-cm
#include "textchannel.h"
#include "callchannel.h"
#include "dbustypes.h"

class MockTextChannel;
class MockCallChannel;
class MockConnectionDBus;

class MockConnection : public Tp::BaseConnection
{
    Q_OBJECT
    Q_DISABLE_COPY(MockConnection)
public:
    MockConnection(const QDBusConnection &dbusConnection,
                    const QString &cmName,
                    const QString &protocolName,
                    const QVariantMap &parameters);

    QStringList inspectHandles(uint handleType, const Tp::UIntList& handles, Tp::DBusError *error);
    Tp::UIntList requestHandles(uint handleType, const QStringList& identifiers, Tp::DBusError* error);
    Tp::BaseChannelPtr createChannel(const QString& channelType, uint targetHandleType,
                                     uint targetHandle, Tp::DBusError *error);
    Tp::ContactAttributesMap getContactAttributes(const Tp::UIntList &handles, const QStringList &ifaces, Tp::DBusError *error);
    uint setPresence(const QString& status, const QString& statusMessage, Tp::DBusError *error);
    void connect(Tp::DBusError *error);
    void setOnline(bool online);

    Tp::BaseConnectionRequestsInterfacePtr requestsIface;
    Tp::BaseConnectionSimplePresenceInterfacePtr simplePresenceIface;
    Tp::BaseConnectionContactsInterfacePtr contactsIface;
    uint newHandle(const QString &identifier);

    uint ensureHandle(const QString &id);
    Tp::BaseChannelPtr createTextChannel(uint targetHandleType,
                                         uint targetHandle, Tp::DBusError *error);
    Tp::BaseChannelPtr createCallChannel(uint targetHandleType,
                                         uint targetHandle, Tp::DBusError *error);

    ~MockConnection();
Q_SIGNALS:
    void messageSent(const QString &message, const QVariantMap &info);

public Q_SLOTS:
    void placeIncomingMessage(const QString &message, const QVariantMap &info);
    void placeCall(const QVariantMap &properties);
    void hangupCall(const QString &callerId);
    void setCallState(const QString &phoneNumber, const QString &state);
    void onTextChannelClosed();
    void onCallChannelClosed();
    void onMessageRead(const QString &id);

private:
    void addMMSToService(const QString &path, const QVariantMap &properties, const QString &servicePath);
    QMap<uint, QString> mHandles;

    QMap<QString, MockTextChannel*> mTextChannels;
    QMap<QString, MockCallChannel*> mCallChannels;
    QMap<QString, QString> mInitialCallStatus;

    QStringList mModems;
    uint mHandleCount;
    Tp::SimplePresence mSelfPresence;
    Tp::SimplePresence mRequestedSelfPresence;

    MockConnectionDBus *mDBus;
};

#endif
