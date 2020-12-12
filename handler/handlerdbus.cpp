/*
 * Copyright (C) 2012-2017 Canonical, Ltd.
 *
 * Authors:
 *  Ugo Riboni <ugo.riboni@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "accountproperties.h"
#include "audiooutput.h"
#include "audioroutemanager.h"
#include "callhandler.h"
#include "handlerdbus.h"
#include "handleradaptor.h"
#include "texthandler.h"
#include "telepathyhelper.h"
#include "protocolmanager.h"
#include <config.h>

// Qt
#include <QtDBus/QDBusConnection>

static const char* DBUS_SERVICE = "com.canonical.TelephonyServiceHandler";
static const char* DBUS_OBJECT_PATH = "/com/canonical/TelephonyServiceHandler";

HandlerDBus::HandlerDBus(QObject* parent) : QObject(parent), mCallIndicatorVisible(false)
{
    qDBusRegisterMetaType<ProtocolList>();
    qDBusRegisterMetaType<ProtocolStruct>();

    connect(CallHandler::instance(),
            SIGNAL(callPropertiesChanged(QString,QVariantMap)),
            SIGNAL(CallPropertiesChanged(QString,QVariantMap)));
    connect(CallHandler::instance(),
            SIGNAL(callHoldingFailed(QString)),
            SIGNAL(CallHoldingFailed(QString)));
    connect(CallHandler::instance(),
            SIGNAL(conferenceCallRequestFinished(bool)),
            SIGNAL(ConferenceCallRequestFinished(bool)));
    connect(ProtocolManager::instance(),
            &ProtocolManager::protocolsChanged, [this]() {
                Q_EMIT ProtocolsChanged(ProtocolManager::instance()->protocols().dbusType());
            });
    connect(AudioRouteManager::instance(),
            SIGNAL(audioOutputsChanged(AudioOutputDBusList)),
            SIGNAL(AudioOutputsChanged(AudioOutputDBusList)));
    connect(AudioRouteManager::instance(),
            SIGNAL(activeAudioOutputChanged(QString)),
            SIGNAL(ActiveAudioOutputChanged(QString)));
}

HandlerDBus::~HandlerDBus()
{
}

QVariantMap HandlerDBus::GetCallProperties(const QString &objectPath)
{
    return CallHandler::instance()->getCallProperties(objectPath);
}

bool HandlerDBus::HasCalls()
{
    return CallHandler::instance()->hasCalls();
}

QStringList HandlerDBus::AccountIds()
{
    return TelepathyHelper::instance()->accountIds();
}

bool HandlerDBus::IsReady()
{
    return TelepathyHelper::instance()->ready();
}

bool HandlerDBus::callIndicatorVisible() const
{
    return mCallIndicatorVisible;
}

void HandlerDBus::setCallIndicatorVisible(bool visible)
{
    mCallIndicatorVisible = visible;
    Q_EMIT CallIndicatorVisibleChanged(visible);
}

ProtocolList HandlerDBus::GetProtocols()
{
    return ProtocolManager::instance()->protocols().dbusType();
}

AllAccountsProperties HandlerDBus::GetAllAccountsProperties()
{
    return AccountProperties::instance()->allProperties();
}

QVariantMap HandlerDBus::GetAccountProperties(const QString &accountId)
{
    return AccountProperties::instance()->accountProperties(accountId);
}

void HandlerDBus::SetAccountProperties(const QString &accountId, const QVariantMap &properties)
{
    AccountProperties::instance()->setAccountProperties(accountId, properties);
}

QString HandlerDBus::registerObject(QObject *object, const QString &path)
{
    QString fullPath = QString("%1/%2").arg(DBUS_OBJECT_PATH, path);
    if (QDBusConnection::sessionBus().registerObject(fullPath, object)) {
        return fullPath;
    }
    return QString::null;
}

void HandlerDBus::unregisterObject(const QString &path)
{
    QDBusConnection::sessionBus().unregisterObject(path);
}

HandlerDBus *HandlerDBus::instance()
{
    static HandlerDBus *self = new HandlerDBus;
    return self;
}

void HandlerDBus::InviteParticipants(const QString &objectPath, const QStringList &participants, const QString &message)
{
    TextHandler::instance()->inviteParticipants(objectPath, participants, message);
}

void HandlerDBus::RemoveParticipants(const QString &objectPath, const QStringList &participants, const QString &message)
{
    TextHandler::instance()->removeParticipants(objectPath, participants, message);
}

void HandlerDBus::LeaveRooms(const QString &accountId, const QString &message)
{
    return TextHandler::instance()->leaveRooms(accountId, message);
}

bool HandlerDBus::LeaveChat(const QString &objectPath, const QString &message)
{
    return TextHandler::instance()->leaveChat(objectPath, message);
}

bool HandlerDBus::DestroyTextChannel(const QString &objectPath)
{
    return TextHandler::instance()->destroyTextChannel(objectPath);
}

bool HandlerDBus::ChangeRoomTitle(const QString &objectPath, const QString &title)
{
    return TextHandler::instance()->changeRoomTitle(objectPath, title);
}

void HandlerDBus::setActiveAudioOutput(const QString &id)
{
    AudioRouteManager::instance()->setActiveAudioOutput(id);
}

QString HandlerDBus::activeAudioOutput() const
{
    return AudioRouteManager::instance()->activeAudioOutput();
}

AudioOutputDBusList HandlerDBus::AudioOutputs() const
{
    return AudioRouteManager::instance()->audioOutputs();
}

bool HandlerDBus::connectToBus()
{
    new TelephonyServiceHandlerAdaptor(this);
    QDBusConnection::sessionBus().registerObject(DBUS_OBJECT_PATH, this);
    return QDBusConnection::sessionBus().registerService(DBUS_SERVICE);
}

QString HandlerDBus::SendMessage(const QString &accountId, const QString &message, const AttachmentList &attachments, const QVariantMap &properties)
{
    return TextHandler::instance()->sendMessage(accountId, message, attachments, properties);
}

void HandlerDBus::AcknowledgeMessages(const QVariantList &messages)
{
    TextHandler::instance()->acknowledgeMessages(messages);
}

QString HandlerDBus::StartChat(const QString &accountId, const QVariantMap &properties)
{
    return TextHandler::instance()->startChat(accountId, properties);
}

void HandlerDBus::AcknowledgeAllMessages(const QVariantMap &properties)
{
    qDebug() << "jezek - HandlerDBus::AcknowledgeAllMessages";
    TextHandler::instance()->acknowledgeAllMessages(properties);
}

void HandlerDBus::RedownloadMessage(const QVariantMap &properties)
{
    qDebug() << "jezek - HandlerDBus::RedownloadMessage";
    TextHandler::instance()->redownloadMessage(properties);
}

void HandlerDBus::StartCall(const QString &number, const QString &accountId)
{
    CallHandler::instance()->startCall(number, accountId);
}

void HandlerDBus::HangUpCall(const QString &objectPath)
{
    CallHandler::instance()->hangUpCall(objectPath);
}

void HandlerDBus::SetHold(const QString &objectPath, bool hold)
{
    CallHandler::instance()->setHold(objectPath, hold);
}

void HandlerDBus::SetMuted(const QString &objectPath, bool muted)
{
    CallHandler::instance()->setMuted(objectPath, muted);
}

void HandlerDBus::SendDTMF(const QString &objectPath, const QString &key)
{
    CallHandler::instance()->sendDTMF(objectPath, key);
}

void HandlerDBus::CreateConferenceCall(const QStringList &objectPaths)
{
    CallHandler::instance()->createConferenceCall(objectPaths);
}

void HandlerDBus::MergeCall(const QString &conferenceObjectPath, const QString &callObjectPath)
{
    CallHandler::instance()->mergeCall(conferenceObjectPath, callObjectPath);
}

void HandlerDBus::SplitCall(const QString &objectPath)
{
    CallHandler::instance()->splitCall(objectPath);
}
