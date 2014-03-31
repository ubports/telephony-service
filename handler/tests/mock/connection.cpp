﻿/**
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

#include <QDebug>

#include <TelepathyQt/Constants>
#include <TelepathyQt/BaseChannel>
#include <TelepathyQt/DBusObject>

// telepathy-mock
#include "connection.h"
#include "phoneutils.h"
#include "protocol.h"
#include "conferencecallchannel.h"

#include "mockconnectiondbus.h"

MockConnection::MockConnection(const QDBusConnection &dbusConnection,
                            const QString &cmName,
                            const QString &protocolName,
                            const QVariantMap &parameters) :
    Tp::BaseConnection(dbusConnection, cmName, protocolName, parameters),
    mHandleCount(0), mConferenceCall(0)
{
    setSelfHandle(newHandle("<SelfHandle>"));

    setConnectCallback(Tp::memFun(this,&MockConnection::connect));
    setInspectHandlesCallback(Tp::memFun(this,&MockConnection::inspectHandles));
    setRequestHandlesCallback(Tp::memFun(this,&MockConnection::requestHandles));
    setCreateChannelCallback(Tp::memFun(this,&MockConnection::createChannel));

    // initialise requests interface (Connection.Interface.Requests)
    requestsIface = Tp::BaseConnectionRequestsInterface::create(this);

    // set requestable text channel properties
    Tp::RequestableChannelClass text;
    text.fixedProperties[TP_QT_IFACE_CHANNEL+".ChannelType"] = TP_QT_IFACE_CHANNEL_TYPE_TEXT;
    text.fixedProperties[TP_QT_IFACE_CHANNEL+".TargetHandleType"]  = Tp::HandleTypeContact;
    text.allowedProperties.append(TP_QT_IFACE_CHANNEL+".TargetHandle");
    text.allowedProperties.append(TP_QT_IFACE_CHANNEL+".TargetID");

    // set requestable call channel properties
    Tp::RequestableChannelClass call;
    call.fixedProperties[TP_QT_IFACE_CHANNEL+".ChannelType"] = TP_QT_IFACE_CHANNEL_TYPE_CALL;
    call.fixedProperties[TP_QT_IFACE_CHANNEL+".TargetHandleType"]  = Tp::HandleTypeContact;
    call.fixedProperties[TP_QT_IFACE_CHANNEL_TYPE_CALL+".InitialAudio"]  = true;
    call.allowedProperties.append(TP_QT_IFACE_CHANNEL+".TargetHandle");
    call.allowedProperties.append(TP_QT_IFACE_CHANNEL+".TargetID");
    call.allowedProperties.append(TP_QT_IFACE_CHANNEL_TYPE_CALL+".InitialAudio");
    call.allowedProperties.append(TP_QT_IFACE_CHANNEL_TYPE_CALL+".InitialVideo");
    call.allowedProperties.append(TP_QT_IFACE_CHANNEL_TYPE_CALL+".InitialAudioName");
    call.allowedProperties.append(TP_QT_IFACE_CHANNEL_TYPE_CALL+".InitialVideoName");
    call.allowedProperties.append(TP_QT_IFACE_CHANNEL_TYPE_CALL+".InitialTransport");
    call.allowedProperties.append(TP_QT_IFACE_CHANNEL_TYPE_CALL+".HardwareStreaming");
    call.allowedProperties.append(TP_QT_IFACE_CHANNEL_INTERFACE_CONFERENCE + QLatin1String(".InitialChannels"));

    requestsIface->requestableChannelClasses << text << call;

    plugInterface(Tp::AbstractConnectionInterfacePtr::dynamicCast(requestsIface));

    // init presence interface
    simplePresenceIface = Tp::BaseConnectionSimplePresenceInterface::create();
    simplePresenceIface->setSetPresenceCallback(Tp::memFun(this,&MockConnection::setPresence));
    plugInterface(Tp::AbstractConnectionInterfacePtr::dynamicCast(simplePresenceIface));

    // Set Presence
    Tp::SimpleStatusSpec presenceOnline;
    presenceOnline.type = Tp::ConnectionPresenceTypeAvailable;
    presenceOnline.maySetOnSelf = true;
    presenceOnline.canHaveMessage = false;

    Tp::SimpleStatusSpec presenceOffline;
    presenceOffline.type = Tp::ConnectionPresenceTypeOffline;
    presenceOffline.maySetOnSelf = false;
    presenceOffline.canHaveMessage = false;

    Tp::SimpleStatusSpecMap statuses;
    statuses.insert(QLatin1String("available"), presenceOnline);
    statuses.insert(QLatin1String("offline"), presenceOffline);

    simplePresenceIface->setStatuses(statuses);
    mSelfPresence.type = Tp::ConnectionPresenceTypeOffline;
    mRequestedSelfPresence.type = Tp::ConnectionPresenceTypeOffline;

    contactsIface = Tp::BaseConnectionContactsInterface::create();
    contactsIface->setGetContactAttributesCallback(Tp::memFun(this,&MockConnection::getContactAttributes));
    contactsIface->setContactAttributeInterfaces(QStringList()
                                                 << TP_QT_IFACE_CONNECTION
                                                 << TP_QT_IFACE_CONNECTION_INTERFACE_SIMPLE_PRESENCE);
    plugInterface(Tp::AbstractConnectionInterfacePtr::dynamicCast(contactsIface));

    mDBus = new MockConnectionDBus(this);

    setOnline(true);
}

void MockConnection::addMMSToService(const QString &path, const QVariantMap &properties, const QString &servicePath)
{
    qDebug() << "addMMSToService " << path << properties << servicePath;
#if 0
    FIXME: re-implement
    MMSDMessage *msg = new MMSDMessage(path, properties);
    QObject::connect(msg, SIGNAL(propertyChanged(QString,QVariant)), SLOT(onMMSPropertyChanged(QString,QVariant)));
    mServiceMMSList[servicePath].append(msg);
    if (properties["Status"] ==  "received") {
        const QString normalizedNumber = PhoneUtils::normalizePhoneNumber(properties["Sender"].toString());
        // check if there is an open channel for this number and use it
        Q_FOREACH(const QString &phoneNumber, mTextChannels.keys()) {
            if (PhoneUtils::comparePhoneNumbers(normalizedNumber, phoneNumber)) {
                qDebug() << "existing channel" << mTextChannels[phoneNumber];
                mTextChannels[phoneNumber]->mmsReceived(path, properties);
                return;
            }
        }

        Tp::DBusError error;
        bool yours;
        qDebug() << "new handle" << normalizedNumber;
        uint handle = newHandle(normalizedNumber);
        ensureChannel(TP_QT_IFACE_CHANNEL_TYPE_TEXT,Tp::HandleTypeContact, handle, yours, handle, false, &error);
        if(error.isValid()) {
            qCritical() << "Error creating channel for incoming message " << error.name() << error.message();
            return;
        }
        mTextChannels[normalizedNumber]->mmsReceived(path, properties);
    }
#endif
}

MockConnection::~MockConnection()
{
}

uint MockConnection::setPresence(const QString& status, const QString& statusMessage, Tp::DBusError *error)
{
    qDebug() << "setPresence" << status;
    if (status == "available") {
        mRequestedSelfPresence.type = Tp::ConnectionPresenceTypeAvailable;
    }
    return selfHandle();
}

Tp::ContactAttributesMap MockConnection::getContactAttributes(const Tp::UIntList &handles, const QStringList &ifaces, Tp::DBusError *error)
{
    qDebug() << "getContactAttributes" << handles << ifaces;
    Tp::ContactAttributesMap attributesMap;
    QVariantMap attributes;
    Q_FOREACH(uint handle, handles) {
        attributes[TP_QT_IFACE_CONNECTION+"/contact-id"] = inspectHandles(Tp::HandleTypeContact, Tp::UIntList() << handle, error).at(0);
        if (ifaces.contains(TP_QT_IFACE_CONNECTION_INTERFACE_SIMPLE_PRESENCE)) {
            attributes[TP_QT_IFACE_CONNECTION_INTERFACE_SIMPLE_PRESENCE+"/presence"] = QVariant::fromValue(mSelfPresence);
        }
        attributesMap[handle] = attributes;
    }
    return attributesMap;
}

void MockConnection::setOnline(bool online)
{
    qDebug() << "setOnline" << online;
    Tp::SimpleContactPresences presences;
    if (online) {
        mSelfPresence.status = "available";
        mSelfPresence.statusMessage = "";
        mSelfPresence.type = Tp::ConnectionPresenceTypeAvailable;
    } else {
        mSelfPresence.status = "offline";
        mSelfPresence.statusMessage = "";
        mSelfPresence.type = Tp::ConnectionPresenceTypeOffline;
    }
    presences[selfHandle()] = mSelfPresence;
    simplePresenceIface->setPresences(presences);
}

uint MockConnection::newHandle(const QString &identifier)
{
    mHandles[++mHandleCount] = identifier;
    return mHandleCount;
}

QMap<QString, MockCallChannel *> MockConnection::callChannels()
{
    return mCallChannels;
}

QStringList MockConnection::inspectHandles(uint handleType, const Tp::UIntList& handles, Tp::DBusError *error)
{
    QStringList identifiers;

    if( handleType != Tp::HandleTypeContact ) {
        error->set(TP_QT_ERROR_INVALID_ARGUMENT,"Not supported");
        return QStringList();
    }

    qDebug() << "MockConnection::inspectHandles " << handles;
    Q_FOREACH( uint handle, handles) {
        if (mHandles.keys().contains(handle)) {
            identifiers.append(mHandles.value(handle));
        } else {
            error->set(TP_QT_ERROR_INVALID_HANDLE, "Handle not found");
            return QStringList();
        }
    }
    qDebug() << "MockConnection::inspectHandles " << identifiers;
    return identifiers;
}

void MockConnection::connect(Tp::DBusError *error) {
    qDebug() << "MockConnection::connect";
    setStatus(Tp::ConnectionStatusConnected, Tp::ConnectionStatusReasonRequested);
}

Tp::UIntList MockConnection::requestHandles(uint handleType, const QStringList& identifiers, Tp::DBusError* error)
{
    qDebug() << "requestHandles";
    Tp::UIntList handles;

    if( handleType != Tp::HandleTypeContact ) {
        error->set(TP_QT_ERROR_INVALID_ARGUMENT, "Not supported");
        return Tp::UIntList();
    }

    Q_FOREACH( const QString& identifier, identifiers) {
        if (mHandles.values().contains(identifier)) {
            handles.append(mHandles.key(identifier));
        } else {
            handles.append(newHandle(identifier));
        }
    }
    qDebug() << "requestHandles" << handles;
    return handles;
}

Tp::BaseChannelPtr MockConnection::createTextChannel(uint targetHandleType,
                                               uint targetHandle, Tp::DBusError *error)
{
    Q_UNUSED(targetHandleType);
    Q_UNUSED(error);

    QString requestedId = mHandles.value(targetHandle);

    if (mTextChannels.contains(requestedId)) {
        return mTextChannels[requestedId]->baseChannel();
    }

    MockTextChannel *channel = new MockTextChannel(this, requestedId, targetHandle);
    QObject::connect(channel, SIGNAL(messageRead(QString)), SLOT(onMessageRead(QString)));
    QObject::connect(channel, SIGNAL(destroyed()), SLOT(onTextChannelClosed()));
    QObject::connect(channel, SIGNAL(messageSent(QString,QVariantMap)), SIGNAL(messageSent(QString,QVariantMap)));
    qDebug() << channel;
    mTextChannels[requestedId] = channel;
    return channel->baseChannel();
}

void MockConnection::onMessageRead(const QString &id)
{
    // FIXME: implement
}

void MockConnection::onConferenceCallChannelClosed()
{
    if (mConferenceCall) {
        mConferenceCall = NULL;
    }
}

void MockConnection::onCallChannelSplitted()
{
    MockCallChannel *channel = qobject_cast<MockCallChannel*>(sender());
    Q_EMIT channelSplitted(channel->baseChannel()->objectPath());
    Q_EMIT channelSplitted(QDBusObjectPath(channel->baseChannel()->objectPath()));
}

Tp::BaseChannelPtr MockConnection::createCallChannel(uint targetHandleType, uint targetHandle,
                                                     const QVariantMap &hints, Tp::DBusError *error)
{
    Q_UNUSED(targetHandleType);

    QString requestedId = mHandles.value(targetHandle);

    if (hints.contains(TP_QT_IFACE_CHANNEL_INTERFACE_CONFERENCE + QLatin1String(".InitialChannels")) &&
        targetHandleType == Tp::HandleTypeNone && targetHandle == 0) {
        // conference call request
        if (mConferenceCall) {
            error->set(TP_QT_ERROR_NOT_AVAILABLE, "Conference call already exists");
            return Tp::BaseChannelPtr();
        }

        QDBusArgument arg = hints[TP_QT_IFACE_CHANNEL_INTERFACE_CONFERENCE + QLatin1String(".InitialChannels")].value<QDBusArgument>();
        QList<QDBusObjectPath> channels;
        arg >> channels;
        if (!channels.isEmpty()) {
            mConferenceCall = new MockConferenceCallChannel(this, channels);
            QObject::connect(mConferenceCall, SIGNAL(destroyed()), SLOT(onConferenceCallChannelClosed()));
            QObject::connect(mConferenceCall, SIGNAL(channelMerged(QString)), SIGNAL(channelMerged(QString)));

            // the object path is only availabe after we return to the event loop, so emit the conferenceCreated signal
            // only after that.
            QObject::connect(mConferenceCall, &MockConferenceCallChannel::initialized, [this]() {
                Q_EMIT conferenceCreated(mConferenceCall->baseChannel()->objectPath());
            });
            return mConferenceCall->baseChannel();
        }
        error->set(TP_QT_ERROR_NOT_AVAILABLE, "Impossible to merge calls");
        return Tp::BaseChannelPtr();
    }

    Q_FOREACH(const QString &id, mCallChannels.keys()) {
        if (id == requestedId) {
            return mCallChannels[id]->baseChannel();
        }
    }

    QString state = "dialing";
    if (mInitialCallStatus.contains(requestedId)) {
        state = mInitialCallStatus.take(requestedId);
    }

    mCallChannels[requestedId] = new MockCallChannel(this, requestedId, state, targetHandle);
    QObject::connect(mCallChannels[requestedId], SIGNAL(destroyed()), SLOT(onCallChannelClosed()));
    QObject::connect(mCallChannels[requestedId], SIGNAL(callStateChanged(MockCallChannel*,QString)), SLOT(onCallStateChanged(MockCallChannel*,QString)));
    QObject::connect(mCallChannels[requestedId], SIGNAL(splitted()), SLOT(onCallChannelSplitted()));

    qDebug() << mCallChannels[requestedId];

    if (!mIncomingCalls.contains(requestedId)) {
        Q_EMIT callReceived(requestedId);
    }
    return mCallChannels[requestedId]->baseChannel();
}

Tp::BaseChannelPtr MockConnection::createChannel(const QString& channelType, uint targetHandleType,
                                               uint targetHandle, const QVariantMap &hints, Tp::DBusError *error)
{
    qDebug() << "MockConnection::createChannel" << targetHandle;
    if (mSelfPresence.type != Tp::ConnectionPresenceTypeAvailable) {
        error->set(TP_QT_ERROR_NETWORK_ERROR, "No network available");
        return Tp::BaseChannelPtr();
    }

    if (channelType == TP_QT_IFACE_CHANNEL_TYPE_TEXT) {
        return createTextChannel(targetHandleType, targetHandle, error);
    } else if (channelType == TP_QT_IFACE_CHANNEL_TYPE_CALL) {
        return createCallChannel(targetHandleType, targetHandle, hints, error);
    } else {
        error->set(TP_QT_ERROR_NOT_IMPLEMENTED, "Channel type not available");
    }

    return Tp::BaseChannelPtr();
}

void MockConnection::placeIncomingMessage(const QString &message, const QVariantMap &info)
{
    const QString sender = info["Sender"].toString();
    // check if there is an open channel for this sender and use it
    Q_FOREACH(const QString &id, mTextChannels.keys()) {
        if (id == sender) {
            mTextChannels[id]->messageReceived(message, info);
            return;
        }
    }

    Tp::DBusError error;
    bool yours;
    uint handle = newHandle(sender);
    ensureChannel(TP_QT_IFACE_CHANNEL_TYPE_TEXT,Tp::HandleTypeContact, handle, yours, handle, false, QVariantMap(), &error);
    if(error.isValid()) {
        qWarning() << "Error creating channel for incoming message" << error.name() << error.message();
        return;
    }
    mTextChannels[sender]->messageReceived(message, info);
}

void MockConnection::onTextChannelClosed()
{
    MockTextChannel *channel = static_cast<MockTextChannel*>(sender());
    if (channel) {
        QString key = mTextChannels.key(channel);
        qDebug() << "text channel closed for number " << key;
        mTextChannels.remove(key);
    }
}

void MockConnection::onCallChannelClosed()
{
    qDebug() << "onCallChannelClosed()";
    MockCallChannel *channel = static_cast<MockCallChannel*>(sender());
    if (channel) {
        QString key = mCallChannels.key(channel);
        qDebug() << "call channel closed for number " << key;
        mCallChannels.remove(key);
        mIncomingCalls.removeAll(key);
        Q_EMIT callEnded(key);
    }
}

void MockConnection::onCallChannelDestroyed()
{
    // FIXME: implement
}

void MockConnection::onCallStateChanged(MockCallChannel *channel, const QString &state)
{
    const QString key = mCallChannels.key(channel);
    if (key.isEmpty()) {
        return;
    }

    Q_EMIT callStateChanged(key, channel->objectPath(), state);
}

uint MockConnection::ensureHandle(const QString &id)
{
    if (mHandles.values().contains(id)) {
        return mHandles.key(id);
    }
    return newHandle(id);
}

QString MockConnection::placeCall(const QVariantMap &properties)
{
    qDebug() << "new call" << properties;

    bool yours;
    Tp::DBusError error;
    QString callerId = properties["Caller"].toString();
    QString state = properties["State"].toString();

    if (mCallChannels.contains(callerId)) {
        return mCallChannels[callerId]->objectPath();
    }

    uint handle = ensureHandle(callerId);
    uint initiatorHandle = 0;
    if (state == "incoming" || state == "waiting") {
        initiatorHandle = handle;
    } else {
        initiatorHandle = selfHandle();
    }

    qDebug() << "initiatorHandle " <<initiatorHandle;
    qDebug() << "handle" << handle;

    mInitialCallStatus[callerId] = state;
    mIncomingCalls.append(callerId);

    Tp::BaseChannelPtr channel  = ensureChannel(TP_QT_IFACE_CHANNEL_TYPE_CALL, Tp::HandleTypeContact, handle, yours, initiatorHandle, false, QVariantMap(), &error);
    if (error.isValid() || channel.isNull()) {
        qWarning() << "error creating the channel " << error.name() << error.message();
        return QString();
    }

    return channel->objectPath();
}

void MockConnection::hangupCall(const QString &callerId)
{
    if (!mCallChannels.contains(callerId)) {
        return;
    }

    mCallChannels[callerId]->setCallState("disconnected");
    mIncomingCalls.removeAll(callerId);
}

void MockConnection::setCallState(const QString &phoneNumber, const QString &state)
{
    if (!mCallChannels.contains(phoneNumber)) {
        return;
    }

    mCallChannels[phoneNumber]->setCallState(state);
}
