/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "callhandler.h"
#include "contactmodel.h"
#include "telepathyhelper.h"
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>

#define CANONICAL_IFACE_TELEPHONY "com.canonical.Telephony"
#define TELEPATHY_MUTE_IFACE "org.freedesktop.Telepathy.Call1.Interface.Mute"
#define TELEPATHY_SPEAKER_IFACE "com.canonical.Telephony.Speaker"
#define DBUS_PROPERTIES_IFACE "org.freedesktop.DBus.Properties"
#define PROPERTY_SPEAKERMODE "SpeakerMode"

typedef QMap<QString, QVariant> dbusQMap;
Q_DECLARE_METATYPE(dbusQMap)

CallHandler *CallHandler::instance()
{
    static CallHandler *self = new CallHandler();
    return self;
}

CallHandler::CallHandler(QObject *parent)
: QObject(parent)
{
}

void CallHandler::startCall(const QString &phoneNumber)
{
    // check if we are already talking to that phone number
    if (!existingCall(phoneNumber).isNull()) {
        return;
    }

    // Request the contact to start audio call
    Tp::AccountPtr account = TelepathyHelper::instance()->account();
    if (account->connection() == NULL) {
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

    QDBusInterface speakerInterface(channel->busName(), channel->objectPath(), TELEPATHY_SPEAKER_IFACE);
    speakerInterface.call("turnOnSpeaker", enabled);
}

void CallHandler::sendDTMF(const QString &objectPath, const QString &key)
{
    Tp::CallChannelPtr channel = callFromObjectPath(objectPath);
    if (channel.isNull()) {
        return;
    }

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
}

void CallHandler::onCallChannelAvailable(Tp::CallChannelPtr channel)
{
    channel->accept();

    // check if the channel has the speakermode property
    QDBusInterface callChannelIface(channel->busName(), channel->objectPath(), DBUS_PROPERTIES_IFACE);
    QDBusMessage reply = callChannelIface.call("GetAll", TELEPATHY_SPEAKER_IFACE);
    QVariantList args = reply.arguments();
    QMap<QString, QVariant> map = qdbus_cast<QMap<QString, QVariant> >(args[0]);
    channel->setProperty("hasSpeakerProperty", map.contains(PROPERTY_SPEAKERMODE));

    connect(channel.data(),
            SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            SLOT(onCallChannelInvalidated()));

    mCallChannels.append(channel);
}

void CallHandler::onContactsAvailable(Tp::PendingOperation *op)
{
    Tp::PendingContacts *pc = qobject_cast<Tp::PendingContacts*>(op);

    if (!pc) {
        qCritical() << "The pending object is not a Tp::PendingContacts";
        return;
    }

    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    // start call to the contacts
    Q_FOREACH(Tp::ContactPtr contact, pc->contacts()) {
        account->ensureAudioCall(contact, QLatin1String("audio"), QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".PhoneAppHandler");

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

Tp::CallChannelPtr CallHandler::existingCall(const QString &phoneNumber)
{
    Tp::CallChannelPtr channel;
    Q_FOREACH(const Tp::CallChannelPtr &ch, mCallChannels) {
        if (ContactModel::comparePhoneNumbers(ch->targetContact()->id(), phoneNumber)) {
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
