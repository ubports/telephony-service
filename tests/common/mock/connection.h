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
#include "emergencymodeiface.h"
#include "ussdiface.h"
#include "voicemailiface.h"

class MockTextChannel;
class MockCallChannel;
class MockConnectionDBus;
class MockConferenceCallChannel;

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
    Tp::BaseChannelPtr createChannel(const QVariantMap& request, Tp::DBusError *error);
    Tp::ContactAttributesMap getContactAttributes(const Tp::UIntList &handles, const QStringList &ifaces, Tp::DBusError *error);
    uint setPresence(const QString& status, const QString& statusMessage, Tp::DBusError *error);
    uint setPresenceFail(const QString& status, const QString& statusMessage, Tp::DBusError *error);
    void connect(Tp::DBusError *error);
    void setOnline(bool online);

    Tp::BaseConnectionRequestsInterfacePtr requestsIface;
    Tp::BaseConnectionSimplePresenceInterfacePtr simplePresenceIface;
    Tp::BaseConnectionContactsInterfacePtr contactsIface;
    uint newHandle(const QString &identifier);
    QMap<QString, MockCallChannel*> callChannels();

    // phone custom interfaces
    BaseConnectionEmergencyModeInterfacePtr emergencyModeIface;
    BaseConnectionVoicemailInterfacePtr voicemailIface;
    BaseConnectionUSSDInterfacePtr supplementaryServicesIface;

    uint ensureHandle(const QString &id);
    Tp::BaseChannelPtr createTextChannel(uint targetHandleType,
                                         uint targetHandle,
                                         const QVariantMap &hints,
                                         Tp::DBusError *error);
    Tp::BaseChannelPtr createCallChannel(uint targetHandleType,
                                         uint targetHandle,
                                         const QVariantMap &hints,
                                         Tp::DBusError *error);

    ~MockConnection();

    QString placeCall(const QVariantMap &properties);

    QStringList emergencyNumbers(Tp::DBusError *error);
    void setEmergencyNumbers(const QStringList &emergencyNumbers);
    void setCountryCode(const QString &countryCode);

    bool voicemailIndicator(Tp::DBusError *error);
    void setVoicemailIndicator(bool visible);
    QString voicemailNumber(Tp::DBusError *error);
    void setVoicemailNumber(const QString &number);
    uint voicemailCount(Tp::DBusError *error);
    void setVoicemailCount(int count);
    void USSDInitiate(const QString &command, Tp::DBusError *error);
    void USSDRespond(const QString &reply, Tp::DBusError *error);
    void USSDCancel(Tp::DBusError *error);
    QString serial();

    // FIXME: there is a problem in telepathy-qt that connection object paths and services are not properly unregistered
    // and thus if we gain the same memory address (and thus the same pointer) every time we reconnect, there might be some problems
    QString uniqueName() const;

Q_SIGNALS:
    void messageRead(const QString &messageId);
    void messageSent(const QString &message, const QVariantMap &info);
    void callReceived(const QString &callerId);
    void callEnded(const QString &callerId);
    void callStateChanged(const QString &callerId, const QString &objectPath, const QString &state);

    void conferenceCreated(const QString &objectPath);
    void channelMerged(const QString &objectPath);
    void channelSplitted(const QString &objectPath);
    void channelSplitted(const QDBusObjectPath &objectPath);

    // USSD notifications
    void ussdInitiateCalled(const QString &command);
    void ussdRespondCalled(const QString &reply);
    void ussdCancelCalled();

public Q_SLOTS:
    void placeIncomingMessage(const QString &message, const QVariantMap &info);
    void hangupCall(const QString &callerId);
    void setCallState(const QString &phoneNumber, const QString &state);
    void onTextChannelClosed();
    void onCallChannelClosed();
    void onCallChannelDestroyed();
    void onCallStateChanged(MockCallChannel *channel, const QString &state);
    void onMessageRead(const QString &id);

    void onConferenceCallChannelClosed();
    void onCallChannelSplitted();

private:
    void addMMSToService(const QString &path, const QVariantMap &properties, const QString &servicePath);

    MockTextChannel *textChannelForRecipients(const QStringList &recipients);

    QMap<uint, QString> mHandles;
    Tp::SimpleStatusSpecMap mStatuses;

    QList<MockTextChannel*> mTextChannels;
    QMap<QString, MockCallChannel*> mCallChannels;
    QMap<QString, QString> mInitialCallStatus;

    QStringList mModems;
    Tp::SimplePresence mSelfPresence;

    MockConnectionDBus *mDBus;
    QStringList mIncomingCalls;
    MockConferenceCallChannel *mConferenceCall;

    QStringList mEmergencyNumbers;
    QString mCountryCode;
    int mVoicemailCount;
    bool mVoicemailIndicator;
    QString mVoicemailNumber;
};

#endif
