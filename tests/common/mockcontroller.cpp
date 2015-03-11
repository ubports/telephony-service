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
    connect(&mMockInterface, SIGNAL(MessageRead(QString)),
            this, SIGNAL(messageRead(QString)));
    connect(&mMockInterface, SIGNAL(MessageSent(QString, QVariantMap)),
            this, SIGNAL(messageSent(QString, QVariantMap)));
    connect(&mMockInterface, SIGNAL(CallReceived(QString)),
            this, SIGNAL(callReceived(QString)));
    connect(&mMockInterface, SIGNAL(CallEnded(QString)),
            this, SIGNAL(callEnded(QString)));
    connect(&mMockInterface, SIGNAL(CallStateChanged(QString, QString, QString)),
            this, SIGNAL(callStateChanged(QString,QString,QString)));
    connect(&mMockInterface, SIGNAL(ConferenceCreated(QString)),
            this, SIGNAL(conferenceCreated(QString)));
    connect(&mMockInterface, SIGNAL(ChannelMerged(QString)),
            this, SIGNAL(channelMerged(QString)));
    connect(&mMockInterface, SIGNAL(ChannelSplitted(QString)),
            this, SIGNAL(channelSplitted(QString)));
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

void MockController::setOnline(bool online)
{
    mMockInterface.call("SetOnline", online);
}

void MockController::setVoicemailNumber(const QString &number)
{
    mMockInterface.call("SetVoicemailNumber", number);
}

void MockController::setVoicemailIndicator(bool active)
{
    mMockInterface.call("SetVoicemailIndicator", active);
}

void MockController::setVoicemailCount(int count)
{
    mMockInterface.call("SetVoicemailCount", count);
}

void MockController::setEmergencyNumbers(const QStringList &numbers)
{
    mMockInterface.call("SetEmergencyNumbers", numbers);
}

QString MockController::serial()
{
    QDBusReply<QString> reply = mMockInterface.call("Serial");
    if (!reply.isValid()) {
        return QString::null;
    }

    return reply.value();
}
