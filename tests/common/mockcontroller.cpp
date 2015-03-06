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
 * Authors: 
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 */

#include "mockcontroller.h"
#include <QDBusReply>

static const QString mockService("com.canonical.MockConnection");
static const QString mockObject("/com/canonical/MockConnection/%1");
static const QString mockInterface("com.canonical.MockConnection");

MockController::MockController(const QString &protocol, QObject *parent) :
    QObject(parent), mProtocol(protocol), mMockObject(mockObject.arg(protocol)),
    mMockInterface(mockService, mockObject.arg(protocol), mockInterface)
{
    QDBusConnection::sessionBus().connect(mockService, mMockObject, mockInterface, "MessageSent", this, SIGNAL(messageSent(QString, QVariantMap)));
    QDBusConnection::sessionBus().connect(mockService, mMockObject, mockInterface, "CallReceived", this, SIGNAL(callReceived(QString)));
    QDBusConnection::sessionBus().connect(mockService, mMockObject, mockInterface, "CallEnded", this, SIGNAL(callEnded(QString)));
    QDBusConnection::sessionBus().connect(mockService, mMockObject, mockInterface, "CallStateChanged", this, SIGNAL(callStateChanged(QString,QString,QString)));
    QDBusConnection::sessionBus().connect(mockService, mMockObject, mockInterface, "ConferenceCreated", this, SIGNAL(conferenceCreated(QString)));
    QDBusConnection::sessionBus().connect(mockService, mMockObject, mockInterface, "ChannelMerged", this, SIGNAL(channelMerged(QString)));
    QDBusConnection::sessionBus().connect(mockService, mMockObject, mockInterface, "ChannelSplitted", this, SIGNAL(channelSplitted(QString)));
}

void MockController::placeIncomingMessage(const QString &message, const QVariantMap &properties)
{
    mMockInterface.call("PlaceIncomingMessage", message, properties);
}

QString MockController::placeCall(const QVariantMap &properties)
{
    QDBusReply<QString> reply = mMockInterface.call("PlaceCall", properties);
    return reply;
}

void MockController::hangupCall(const QString &callerId)
{
    mMockInterface.call("HangupCall", callerId);
}

void MockController::setCallState(const QString &phoneNumber, const QString &state)
{
    mMockInterface.call("SetCallState", phoneNumber, state);
}
