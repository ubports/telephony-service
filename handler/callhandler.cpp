/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "callhandler.h"
#include "phoneutils.h"
#include "telepathyhelper.h"
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>

#define TELEPATHY_MUTE_IFACE "org.freedesktop.Telepathy.Call1.Interface.Mute"
#define DBUS_PROPERTIES_IFACE "org.freedesktop.DBus.Properties"
#define PROPERTY_SPEAKERMODE "SpeakerMode"

typedef QMap<QString, QVariant> dbusQMap;
Q_DECLARE_METATYPE(dbusQMap)

CallHandler *CallHandler::instance()
{
    static CallHandler *self = new CallHandler();
    return self;
}

QVariantMap CallHandler::getCallProperties(const QString &objectPath)
{
    QVariantMap properties;
    Tp::CallChannelPtr channel = callFromObjectPath(objectPath);
    if (!channel) {
        return properties;
    }

    QVariant property = channel->property("timestamp");
    if (property.isValid()) {
        properties["timestamp"] = property;
    }

    property = channel->property("activeTimestamp");
    if (property.isValid()) {
        properties["activeTimestamp"] = property;
    }
    property = channel->property("dtmfString");
    if (property.isValid()) {
        properties["dtmfString"] = property;
    }

    return properties;
}

bool CallHandler::hasCalls() const
{
    bool hasActiveCalls = false;

    Q_FOREACH(const Tp::CallChannelPtr channel, mCallChannels) {
        Tp::AccountPtr account = TelepathyHelper::instance()->accountForConnection(channel->connection());
        bool incoming = channel->initiatorContact() != account->connection()->selfContact();
        bool dialing = !incoming && (channel->callState() == Tp::CallStateInitialised);
        bool active = channel->callState() == Tp::CallStateActive;

        if (dialing || active) {
            hasActiveCalls = true;
            break;
        }
    }

    return hasActiveCalls;
}

CallHandler::CallHandler(QObject *parent)
: QObject(parent)
{
}

void CallHandler::startCall(const QString &phoneNumber, const QString &accountId)
{
    // Request the contact to start audio call
    Tp::AccountPtr account = TelepathyHelper::instance()->accountForId(accountId);
    if (!account || account->connection() == NULL) {
        return;
    }

    connect(account->connection()->contactManager()->contactsForIdentifiers(QStringList() << phoneNumber),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onContactsAvailable(Tp::PendingOperation*)));
}

void CallHandler::hangUpCall(const QString &objectPath)
{
    Tp::CallChannelPtr channel = callFromObjectPath(objectPath);
    if (channel.isNull()) {
        return;
    }

    Tp::PendingOperation *pending = channel->hangup();
    mClosingChannels[pending] = channel;
    connect(pending,
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onCallHangupFinished(Tp::PendingOperation*)));
}

void CallHandler::setHold(const QString &objectPath, bool hold)
{
    Tp::CallChannelPtr channel = callFromObjectPath(objectPath);
    if (channel.isNull()) {
        return;
    }

    channel->requestHold(hold);
}

void CallHandler::setMuted(const QString &objectPath, bool muted)
{
    Tp::CallChannelPtr channel = callFromObjectPath(objectPath);
    if (channel.isNull()) {
        return;
    }

    // FIXME: replace by a proper TpQt implementation of mute
    QDBusInterface muteInterface(channel->busName(), channel->objectPath(), TELEPATHY_MUTE_IFACE);
    muteInterface.call("RequestMuted", muted);
}

void CallHandler::setSpeakerMode(const QString &objectPath, bool enabled)
{
    Tp::CallChannelPtr channel = callFromObjectPath(objectPath);
    if (channel.isNull() || !channel->property("hasSpeakerProperty").toBool()) {
        return;
    }

    QDBusInterface speakerInterface(channel->busName(), channel->objectPath(), CANONICAL_TELEPHONY_SPEAKER_IFACE);
    speakerInterface.call("turnOnSpeaker", enabled);
}

void CallHandler::sendDTMF(const QString &objectPath, const QString &key)
{
    Tp::CallChannelPtr channel = callFromObjectPath(objectPath);
    if (channel.isNull()) {
        return;
    }

    // save the dtmfString to send to clients that request it
    QString dtmfString = channel->property("dtmfString").toString();
    dtmfString += key;
    channel->setProperty("dtmfString", dtmfString);

    Q_FOREACH(const Tp::CallContentPtr &content, channel->contents()) {
        if (content->supportsDTMF()) {
            bool ok;
            Tp::DTMFEvent event = (Tp::DTMFEvent)key.toInt(&ok);
            if (!ok) {
                 if (!key.compare("*")) {
                     event = Tp::DTMFEventAsterisk;
                 } else if (!key.compare("#")) {
                     event = Tp::DTMFEventHash;
                 } else {
                     qWarning() << "Tone not recognized. DTMF failed";
                     return;
                 }
            }
            content->startDTMFTone(event);
        }
    }

    Q_EMIT callPropertiesChanged(channel->objectPath(), getCallProperties(channel->objectPath()));
}

void CallHandler::createConferenceCall(const QStringList &objectPaths)
{
    QList<Tp::ChannelPtr> calls;
    Tp::AccountPtr account;
    Q_FOREACH(const QString &objectPath, objectPaths) {
        Tp::CallChannelPtr call = callFromObjectPath(objectPath);
        if (!call) {
            qWarning() << "Could not find a call channel for objectPath:" << objectPath;
            return;
        }

        if (account.isNull()) {
            account = TelepathyHelper::instance()->accountForConnection(call->connection());
        }

        // make sure all call channels belong to the same connection
        if (call->connection() != account->connection()) {
            qWarning() << "It is not possible to merge channels from different accounts.";
            return;
        }
        calls.append(call);
    }

    if (calls.isEmpty() || account.isNull()) {
        qWarning() << "The list of calls was empty. Failed to create a conference.";
        return;
    }

    // there is no need to check the pending request. The new channel will arrive at some point.
    account->createConferenceCall(calls, QStringList(), QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler");
}

void CallHandler::mergeCall(const QString &conferenceObjectPath, const QString &callObjectPath)
{
    Tp::CallChannelPtr conferenceChannel = callFromObjectPath(conferenceObjectPath);
    Tp::CallChannelPtr callChannel = callFromObjectPath(callObjectPath);
    if (!conferenceChannel || !callChannel || !conferenceChannel->isConference()) {
        qWarning() << "No valid channels found.";
        return;
    }

    // there is no need to check for the result here.
    conferenceChannel->conferenceMergeChannel(callChannel);
}

void CallHandler::splitCall(const QString &objectPath)
{
    Tp::CallChannelPtr channel = callFromObjectPath(objectPath);
    if (!channel) {
        return;
    }

    // we don't need to check the result of the operation here
    channel->conferenceSplitChannel();
}

void CallHandler::onCallChannelAvailable(Tp::CallChannelPtr channel)
{
    channel->accept();

    // check if the channel has the speakermode property
    QDBusInterface callChannelIface(channel->busName(), channel->objectPath(), DBUS_PROPERTIES_IFACE);
    QDBusMessage reply = callChannelIface.call("GetAll", CANONICAL_TELEPHONY_SPEAKER_IFACE);
    QVariantList args = reply.arguments();
    QMap<QString, QVariant> map = qdbus_cast<QMap<QString, QVariant> >(args[0]);
    channel->setProperty("hasSpeakerProperty", map.contains(PROPERTY_SPEAKERMODE));
    channel->setProperty("timestamp", QDateTime::currentDateTimeUtc());

    if (channel->callState() == Tp::CallStateActive) {
        channel->setProperty("activeTimestamp", QDateTime::currentDateTimeUtc());
    }

    connect(channel.data(),
            SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            SLOT(onCallChannelInvalidated()));
    connect(channel.data(),
            SIGNAL(callStateChanged(Tp::CallState)),
            SLOT(onCallStateChanged(Tp::CallState)));

    mCallChannels.append(channel);
    Q_EMIT callPropertiesChanged(channel->objectPath(), getCallProperties(channel->objectPath()));
}

void CallHandler::onContactsAvailable(Tp::PendingOperation *op)
{
    Tp::PendingContacts *pc = qobject_cast<Tp::PendingContacts*>(op);

    if (!pc) {
        qCritical() << "The pending object is not a Tp::PendingContacts";
        return;
    }

    Tp::AccountPtr account = TelepathyHelper::instance()->accountForConnection(pc->manager()->connection());

    // start call to the contacts
    Q_FOREACH(Tp::ContactPtr contact, pc->contacts()) {
        account->ensureAudioCall(contact, QLatin1String("audio"), QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler");

        // hold the ContactPtr to make sure its refcounting stays bigger than 0
        mContacts[contact->id()] = contact;
    }
}

void CallHandler::onCallHangupFinished(Tp::PendingOperation *op)
{
    if (!mClosingChannels.contains(op)) {
        qCritical() << "Channel for pending hangup not found:" << op;
        return;
    }

    // Do NOT request the channel closing at this point. It will get closed automatically.
    // if you request it to be closed, the CallStateEnded will never be reached and the UI
    // and logging will be broken.
    Tp::CallChannelPtr channel = mClosingChannels.take(op);
    mCallChannels.removeAll(channel);
}

void CallHandler::onCallChannelInvalidated()
{
    Tp::CallChannelPtr channel(qobject_cast<Tp::CallChannel*>(sender()));

    if (channel.isNull()) {
        return;
    }

    mCallChannels.removeAll(channel);
}

void CallHandler::onCallStateChanged(Tp::CallState state)
{
    Tp::CallChannelPtr channel(qobject_cast<Tp::CallChannel*>(sender()));
    if (!channel) {
        return;
    }

    switch (state) {
    case Tp::CallStateActive:
        channel->setProperty("activeTimestamp", QDateTime::currentDateTimeUtc());
        Q_EMIT callPropertiesChanged(channel->objectPath(), getCallProperties(channel->objectPath()));
        break;
    }
}

Tp::CallChannelPtr CallHandler::existingCall(const QString &phoneNumber)
{
    Tp::CallChannelPtr channel;
    Q_FOREACH(const Tp::CallChannelPtr &ch, mCallChannels) {
        if (ch->isConference()) {
            continue;
        }

        if (PhoneUtils::comparePhoneNumbers(ch->targetContact()->id(), phoneNumber)) {
            channel = ch;
            break;
        }
    }

    return channel;
}

Tp::CallChannelPtr CallHandler::callFromObjectPath(const QString &objectPath)
{
    Tp::CallChannelPtr channel;
    Q_FOREACH(const Tp::CallChannelPtr &ch, mCallChannels) {
        if (ch->objectPath() == objectPath) {
            channel = ch;
            break;
        }
    }

    return channel;
}
