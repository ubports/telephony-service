/*
 * Copyright (C) 2012-2017 Canonical, Ltd.
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

#include "accountproperties.h"
#include "callagent.h"
#include "callhandler.h"
#include "telepathyhelper.h"
#include "accountentry.h"
#include "tonegenerator.h"
#include "greetercontacts.h"
#include "phoneutils.h"
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/PendingVariant>
#include <memory>

#define TELEPATHY_MUTE_IFACE "org.freedesktop.Telepathy.Call1.Interface.Mute"
#define DBUS_PROPERTIES_IFACE "org.freedesktop.DBus.Properties"

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
        bool incoming = isIncoming(channel);
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
: QObject(parent),
  mHangupRequested(false)
{
}

void CallHandler::startCall(const QString &targetId, const QString &accountId)
{
    QString finalId = targetId;
    // Request the contact to start audio call
    AccountEntry *accountEntry = TelepathyHelper::instance()->accountForId(accountId);
    if (!accountEntry) {
        return;
    }

    Tp::ConnectionPtr connection = accountEntry->account()->connection();
    if (!connection) {
        return;
    }

    // FIXME: this is a workaround, there might be a better way of handling this.
    // One idea is to implement the Addressing interface on the SIP connection manager such that
    // we can request a handle based on the vCard field "tel"
    if (accountEntry->protocolInfo()->name() == "sip") {
        // check if the phone number needs rewriting
        QVariantMap accountProperties = AccountProperties::instance()->accountProperties(accountId);
        finalId = applyNumberRewritingRules(finalId, accountProperties);

        // replace the numbers by a SIP URI
        QString domain = accountEntry->account()->parameters()["account"].toString();
        if (domain.contains("@")) {
            domain = domain.split("@")[1];

            finalId = QString("sip:%1@%2").arg(PhoneUtils::normalizePhoneNumber(finalId)).arg(domain);
        }
    }

    connect(connection->contactManager()->contactsForIdentifiers(QStringList() << finalId),
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

    Tp::PendingOperation *op = channel->requestHold(hold);
    connect(op, &Tp::PendingOperation::finished, [this, objectPath, op] {
        if (op->isError()) {
            Q_EMIT callHoldingFailed(objectPath);
        }
    });
}

void CallHandler::setMuted(const QString &objectPath, bool muted)
{
    Tp::CallChannelPtr channel = callFromObjectPath(objectPath);
    if (channel.isNull()) {
        return;
    }

    if (channel->handlerStreamingRequired()) {
        CallAgent *agent = mCallAgents[channel.data()];
        if (!agent) {
            return;
        }
        agent->setMute(muted);
    } else {
        // FIXME: replace by a proper TpQt implementation of mute
        QDBusInterface muteInterface(channel->busName(), channel->objectPath(), TELEPATHY_MUTE_IFACE);
        muteInterface.call("RequestMuted", muted);
    }
}

void CallHandler::sendDTMF(const QString &objectPath, const QString &key)
{
    /*
     * play locally (via tone generator) only if we are on a call, or if this is
     * dialpad sounds
     */
    int event = toDTMFEvent(key);
    if (GreeterContacts::instance()->dialpadSoundsEnabled() &&
        !GreeterContacts::instance()->silentMode() && objectPath.isEmpty()
        || !objectPath.isEmpty()) {
        ToneGenerator::instance()->playDTMFTone(event);
    }

    Tp::CallChannelPtr channel = callFromObjectPath(objectPath);
    if (channel.isNull()) {
        return;
    }

    // save the dtmfString to send to clients that request it
    QString dtmfString = channel->property("dtmfString").toString();
    QString pendingDTMF = channel->property("pendingDTMF").toString();
    pendingDTMF += key;
    dtmfString += key;
    channel->setProperty("dtmfString", dtmfString);
    channel->setProperty("pendingDTMF", pendingDTMF);

    // if there is only one pending DTMF event, start playing it
    if (pendingDTMF.length() == 1) {
        playNextDTMFTone(channel);
    }

    Q_EMIT callPropertiesChanged(channel->objectPath(), getCallProperties(channel->objectPath()));
}

void CallHandler::createConferenceCall(const QStringList &objectPaths)
{
    QList<Tp::ChannelPtr> calls;
    AccountEntry *accountEntry = 0;
    Q_FOREACH(const QString &objectPath, objectPaths) {
        Tp::CallChannelPtr call = callFromObjectPath(objectPath);
        if (!call) {
            qWarning() << "Could not find a call channel for objectPath:" << objectPath;
            return;
        }

        if (!accountEntry) {
            accountEntry = TelepathyHelper::instance()->accountForConnection(call->connection());
        }

        // make sure all call channels belong to the same connection
        if (call->connection() != accountEntry->account()->connection()) {
            qWarning() << "It is not possible to merge channels from different accounts.";
            return;
        }
        calls.append(call);
    }

    if (calls.isEmpty() || !accountEntry) {
        qWarning() << "The list of calls was empty. Failed to create a conference.";
        return;
    }

    // there is no need to check the pending request. The new channel will arrive at some point.
    Tp::PendingChannelRequest *pcr = accountEntry->account()->createConferenceCall(calls, QStringList(), QDateTime::currentDateTime(),
                                                                                   TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler");
    connect(pcr, &Tp::PendingChannelRequest::finished, [this, pcr] {
        Q_EMIT conferenceCallRequestFinished(!pcr->isError());
    });
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
    QDBusInterface callChannelIface(channel->busName(), channel->objectPath(), DBUS_PROPERTIES_IFACE);
    channel->setProperty("timestamp", QDateTime::currentDateTimeUtc());

    if (channel->callState() == Tp::CallStateActive) {
        channel->setProperty("activeTimestamp", QDateTime::currentDateTimeUtc());
    } else if (channel->callState() == Tp::CallStatePendingInitiator) {
        channel->accept();
    }

    connect(channel.data(),
            SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            SLOT(onCallChannelInvalidated()));
    connect(channel.data(),
            SIGNAL(callStateChanged(Tp::CallState)),
            SLOT(onCallStateChanged(Tp::CallState)));

    // FIXME: save this to a list
    CallAgent *agent = new CallAgent(channel, this);
    mCallAgents[channel.data()] = agent;

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

    AccountEntry *accountEntry = TelepathyHelper::instance()->accountForConnection(pc->manager()->connection());

    // start call to the contacts
    Q_FOREACH(Tp::ContactPtr contact, pc->contacts()) {
        accountEntry->account()->ensureAudioCall(contact, QLatin1String("audio"), QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler");

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
    if (mCallChannels.count() == 1) {
        mHangupRequested = true;
    }
}

void CallHandler::onCallChannelInvalidated()
{
    Tp::CallChannelPtr channel(qobject_cast<Tp::CallChannel*>(sender()));

    if (channel.isNull()) {
        return;
    }

    mCallChannels.removeAll(channel);
    if (mCallAgents.contains(channel.data())) {
        CallAgent *agent = mCallAgents.take(channel.data());
        agent->deleteLater();
    }

    ToneGenerator::instance()->stopTone();
    if (mCallChannels.isEmpty() && !mHangupRequested) {
        ToneGenerator::instance()->playCallEndedTone();
    }
    mHangupRequested = false;
}

void CallHandler::onCallStateChanged(Tp::CallState state)
{
    Tp::CallChannelPtr channel(qobject_cast<Tp::CallChannel*>(sender()));
    if (!channel) {
        return;
    }

    switch (state) {
    case Tp::CallStatePendingInitiator:
    case Tp::CallStateInitialising:
        if (!isIncoming(channel) && channel->handlerStreamingRequired()) {
            ToneGenerator::instance()->playDialingTone();
        }
        break;
    case Tp::CallStateInitialised:
        if (!isIncoming(channel) && channel->handlerStreamingRequired()) {
            ToneGenerator::instance()->stopTone();
            ToneGenerator::instance()->playRingingTone();
        }
        break;
    case Tp::CallStateActive:
        if (channel->handlerStreamingRequired()) {
            ToneGenerator::instance()->stopTone();
        }
        channel->setProperty("activeTimestamp", QDateTime::currentDateTimeUtc());
        Q_EMIT callPropertiesChanged(channel->objectPath(), getCallProperties(channel->objectPath()));
        break;
    case Tp::CallStateEnded:
        ToneGenerator::instance()->stopTone();
        channel->requestClose();
        break;
    }
}

Tp::CallChannelPtr CallHandler::existingCall(const QString &targetId)
{
    Tp::CallChannelPtr channel;
    Q_FOREACH(const Tp::CallChannelPtr &ch, mCallChannels) {
        if (ch->isConference()) {
            continue;
        }

        AccountEntry *account = TelepathyHelper::instance()->accountForConnection(ch->connection());
        if (!account) {
            continue;
        }

        if (account->compareIds(ch->targetContact()->id(), targetId)) {
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

void CallHandler::playNextDTMFTone(Tp::CallChannelPtr channel)
{
    // the channel might have been closed already
    if (!channel) {
        return;
    }

    QString pendingDTMF = channel->property("pendingDTMF").toString();
    QString key = "";
    if (!pendingDTMF.isEmpty()) {
        key = pendingDTMF[0];
    }

    int event = toDTMFEvent(key);

    Q_FOREACH(const Tp::CallContentPtr &content, channel->contents()) {
        if (content->supportsDTMF()) {

            /* stop any previous DTMF tone before sending the new one*/
            connect(content->stopDTMFTone(), &Tp::PendingOperation::finished, [=](Tp::PendingOperation *op){
                // in case stopDTMFTone, it might mean the service automatically stops the tone,
                // so try playing the next one
                if (op->isError()) {
                    /* send DTMF to network (via telepathy) */
                    if (event >= 0) {
                        content->startDTMFTone((Tp::DTMFEvent)event);
                    }
                    triggerNextDTMFTone(channel);
                    return;
                }

                Tp::Client::CallContentInterfaceDTMFInterface *dtmfInterface = content->interface<Tp::Client::CallContentInterfaceDTMFInterface>();
                Tp::PendingVariant *pv = dtmfInterface->requestPropertyCurrentlySendingTones();
                connect(pv, &Tp::PendingOperation::finished, [=](){
                    bool sendingTones = pv->result().toBool();
                    // if we already stopped sending tones, we can send the next one
                    if (!sendingTones) {
                        /* send DTMF to network (via telepathy) */
                        if (event >= 0) {
                            content->startDTMFTone((Tp::DTMFEvent)event);
                        }
                        triggerNextDTMFTone(channel);
                        return;
                    }

                    // in case the previous tone is not finished, we need to wait for it
                    auto conn = std::make_shared<QMetaObject::Connection>();
                    *conn = connect(dtmfInterface, &Tp::Client::CallContentInterfaceDTMFInterface::StoppedTones, [=](){
                        QObject::disconnect(*conn);

                        /* send DTMF to network (via telepathy) */
                        if (event >= 0) {
                            content->startDTMFTone((Tp::DTMFEvent)event);
                        }
                        triggerNextDTMFTone(channel);
                    });
                });

            });
        }
    }
}

void CallHandler::triggerNextDTMFTone(Tp::CallChannelPtr channel)
{
    QTimer::singleShot(250, [=](){
        QString pendingDTMF = channel->property("pendingDTMF").toString();
        if (pendingDTMF.isEmpty()) {
            return;
        }
        pendingDTMF.remove(0, 1);
        channel->setProperty("pendingDTMF", pendingDTMF);
        playNextDTMFTone(channel);
    });
}

int CallHandler::toDTMFEvent(const QString &key)
{
    bool ok;
    int ev = key.toInt(&ok);
    if (!ok) {
         if (key == "*") {
             ev = Tp::DTMFEventAsterisk;
         } else if (key == "#") {
             ev = Tp::DTMFEventHash;
         } else {
             ev = -1;
         }
    }
    return ev;
}

bool CallHandler::isIncoming(const Tp::CallChannelPtr &channel) const
{
    AccountEntry *accountEntry = TelepathyHelper::instance()->accountForConnection(channel->connection());
    return channel->initiatorContact() != accountEntry->account()->connection()->selfContact();
}

QString CallHandler::applyNumberRewritingRules(const QString &originalNumber, const QVariantMap &properties)
{
    QString finalNumber = originalNumber;
    if (properties.contains("numberRewrite") && properties["numberRewrite"].toBool()) {
        // FIXME: do a proper phone number identification implementation
        // for now consider anything bigger than 6 digits to be a phone number
        if (finalNumber.length() <= 6) {
            return finalNumber;
        }

        QString defaultCountryCode = properties["defaultCountryCode"].toString();
        QString defaultAreaCode = properties["defaultAreaCode"].toString();
        QString removeCharacters = properties["removeCharacters"].toString();
        QString prefix = properties["prefix"].toString();

        if (!defaultCountryCode.startsWith("+")) {
            defaultCountryCode.prepend("+");
        }

        finalNumber = PhoneUtils::getFullNumber(finalNumber, defaultCountryCode, defaultAreaCode);
        finalNumber.remove(removeCharacters);
        finalNumber.prepend(prefix);
    }

    return finalNumber;
}
