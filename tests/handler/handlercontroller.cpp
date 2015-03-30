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
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 */

#include <QStringList>
#include "handlercontroller.h"
#include <QDBusReply>
#include <QDebug>

#define HANDLER_SERVICE "com.canonical.TelephonyServiceHandler"
#define HANDLER_OBJECT "/com/canonical/TelephonyServiceHandler"
#define HANDLER_INTERFACE "com.canonical.TelephonyServiceHandler"

HandlerController *HandlerController::instance()
{
    static HandlerController *self = new HandlerController();
    return self;
}

HandlerController::HandlerController(QObject *parent) :
    QObject(parent),
    mHandlerInterface(HANDLER_SERVICE, HANDLER_OBJECT, HANDLER_INTERFACE)
{
    connect(&mHandlerInterface,
            SIGNAL(CallPropertiesChanged(QString, QVariantMap)),
            SIGNAL(callPropertiesChanged(QString, QVariantMap)));
    connect(&mHandlerInterface,
            SIGNAL(CallIndicatorVisibleChanged(bool)),
            SIGNAL(callIndicatorVisibleChanged(bool)));
}

QVariantMap HandlerController::getCallProperties(const QString &objectPath)
{
    QVariantMap properties;
    QDBusReply<QVariantMap> reply = mHandlerInterface.call("GetCallProperties", objectPath);
    if (reply.isValid()) {
        properties = reply.value();
    }

    return properties;
}

bool HandlerController::callIndicatorVisible()
{
    QDBusInterface handlerPropertiesInterface("com.canonical.TelephonyServiceHandler",
                                              "/com/canonical/TelephonyServiceHandler",
                                              "org.freedesktop.DBus.Properties");
    QDBusReply<QVariantMap> reply = handlerPropertiesInterface.call("GetAll", "com.canonical.TelephonyServiceHandler");
    if (!reply.isValid()) {
        return false;
    }

    QVariantMap map = reply.value();
    return map["CallIndicatorVisible"].toBool();
}

void HandlerController::startCall(const QString &number, const QString &accountId)
{
    mHandlerInterface.call("StartCall", number, accountId);
}

void HandlerController::hangUpCall(const QString &objectPath)
{
    mHandlerInterface.call("HangUpCall", objectPath);
}

void HandlerController::setHold(const QString &objectPath, bool hold)
{
    mHandlerInterface.call("SetHold", objectPath, hold);
}

void HandlerController::setMuted(const QString &objectPath, bool muted)
{
    mHandlerInterface.call("SetMuted", objectPath, muted);
}

void HandlerController::setSpeakerMode(const QString &objectPath, bool enabled)
{
    mHandlerInterface.call("SetSpeakerMode", objectPath, enabled);
}

void HandlerController::sendDTMF(const QString &objectPath, const QString &key)
{
    mHandlerInterface.call("SendDTMF", objectPath, key);
}

void HandlerController::createConferenceCall(const QStringList &objectPaths)
{
    mHandlerInterface.call("CreateConferenceCall", objectPaths);
}

void HandlerController::mergeCall(const QString &conferenceObjectPath, const QString &callObjectPath)
{
    mHandlerInterface.call("MergeCall", conferenceObjectPath, callObjectPath);
}

void HandlerController::splitCall(const QString &objectPath)
{
    mHandlerInterface.call("SplitCall", objectPath);
}

void HandlerController::sendMessage(const QString &number, const QString &message, const QString &accountId)
{
    mHandlerInterface.call("SendMessage", QStringList() << number, message, accountId);
}

void HandlerController::acknowledgeMessages(const QString &number, const QStringList &messageIds, const QString &accountId)
{
    mHandlerInterface.call("AcknowledgeMessages", number, messageIds, accountId);
}

void HandlerController::setCallIndicatorVisible(bool visible)
{
    QDBusInterface handlerPropertiesInterface("com.canonical.TelephonyServiceHandler",
                                              "/com/canonical/TelephonyServiceHandler",
                                              "org.freedesktop.DBus.Properties");
    handlerPropertiesInterface.call("Set",
                                    "com.canonical.TelephonyServiceHandler",
                                    "CallIndicatorVisible", QVariant::fromValue(QDBusVariant(visible)));
}
