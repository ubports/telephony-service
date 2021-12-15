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

#ifndef HANDLERDBUS_H
#define HANDLERDBUS_H

#include <QtCore/QObject>
#include <QtDBus/QDBusContext>
#include "chatmanager.h"
#include "dbustypes.h"
#include "audiooutput.h"

typedef QMap<QString,QVariantMap> AllAccountsProperties;

/**
 * DBus interface for the phone handler
 */
class HandlerDBus : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_PROPERTY(bool CallIndicatorVisible
               READ callIndicatorVisible
               WRITE setCallIndicatorVisible
               NOTIFY CallIndicatorVisibleChanged)

    Q_PROPERTY(QString ActiveAudioOutput
               READ activeAudioOutput
               WRITE setActiveAudioOutput
               NOTIFY ActiveAudioOutputChanged)

public:
    HandlerDBus(QObject* parent=0);
    ~HandlerDBus();

    QVariantMap GetCallProperties(const QString &objectPath);
    bool HasCalls();
    QStringList AccountIds();
    bool IsReady();
    bool callIndicatorVisible() const;
    void setCallIndicatorVisible(bool visible);
    // configuration related
    ProtocolList GetProtocols();
    AllAccountsProperties GetAllAccountsProperties();
    QVariantMap GetAccountProperties(const QString &accountId);
    void SetAccountProperties(const QString &accountId, const QVariantMap &properties);

    QString registerObject(QObject *object, const QString &path);
    void unregisterObject(const QString &path);

    static HandlerDBus *instance();
    QString activeAudioOutput() const;
    void setActiveAudioOutput(const QString &id);

public Q_SLOTS:
    bool connectToBus();

    // messages related
    QString SendMessage(const QString &accountId, const QString &message, const AttachmentList &attachments, const QVariantMap &properties);
    Q_NOREPLY void AcknowledgeMessages(const QVariantList &messages);
    QString StartChat(const QString &accountId, const QVariantMap &properties);
    Q_NOREPLY void AcknowledgeAllMessages(const QVariantMap &properties);
    Q_NOREPLY void RedownloadMessage(const QString &accountId, const QString &threadId, const QString &eventId);
    bool DestroyTextChannel(const QString &objectPath);
    bool ChangeRoomTitle(const QString &objectPath, const QString &title);
    Q_NOREPLY void InviteParticipants(const QString &objectPath, const QStringList &participants, const QString &message);
    Q_NOREPLY void RemoveParticipants(const QString &objectPath, const QStringList &participants, const QString &message);
    bool LeaveChat(const QString &objectPath, const QString &message);
    Q_NOREPLY void LeaveRooms(const QString &accountId, const QString &message);

    // call related
    Q_NOREPLY void StartCall(const QString &number, const QString &accountId);
    Q_NOREPLY void HangUpCall(const QString &objectPath);
    Q_NOREPLY void SetHold(const QString &objectPath, bool hold);
    Q_NOREPLY void SetMuted(const QString &objectPath, bool muted);
    Q_NOREPLY void SendDTMF(const QString &objectPath, const QString &key);

    // conference call related
    Q_NOREPLY void CreateConferenceCall(const QStringList &objectPaths);
    Q_NOREPLY void MergeCall(const QString &conferenceObjectPath, const QString &callObjectPath);
    Q_NOREPLY void SplitCall(const QString &objectPath);
    AudioOutputDBusList AudioOutputs() const;



Q_SIGNALS:
    void onMessageSent(const QString &number, const QString &message);
    void CallPropertiesChanged(const QString &objectPath, const QVariantMap &properties);
    void AccountPropertiesChanged(const QString &accountId, const QVariantMap &properties);
    void CallIndicatorVisibleChanged(bool visible);
    void ConferenceCallRequestFinished(bool succeeded);
    void CallHoldingFailed(const QString &objectPath);
    void ProtocolsChanged(const ProtocolList &protocols);
    void ActiveAudioOutputChanged(const QString &id);
    void AudioOutputsChanged(const AudioOutputDBusList &audioOutputs);

private:
    bool mCallIndicatorVisible;
};

#endif // HANDLERDBUS_H
