/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "callmanager.h"
#include "telepathyhelper.h"

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>

#define ANDROID_DBUS_ADDRESS "com.canonical.Android"
#define ANDROID_TELEPHONY_DBUS_PATH "/com/canonical/android/telephony/Telephony"
#define ANDROID_TELEPHONY_DBUS_IFACE "com.canonical.android.telephony.Telephony"

#define TP_UFA_DBUS_ADDRESS "org.freedesktop.Telepathy.Connection.ufa.ufa.ufa"
#define TP_UFA_DBUS_MUTE_FACE "org.freedesktop.Telepathy.Call1.Interface.Mute"

CallManager::CallManager(QObject *parent)
: QObject(parent)
{
}

bool CallManager::isTalkingToContact(const QString &contactId)
{
    return mChannels.contains(contactId);
}

void CallManager::startCall(const QString &contactId)
{
    if (!mChannels.contains(contactId)) {
        // Request the contact to start audio call
        Tp::AccountPtr account = TelepathyHelper::instance()->account();
        connect(account->connection()->contactManager()->contactsForIdentifiers(QStringList() << contactId),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onContactsAvailable(Tp::PendingOperation*)));
    }
}

void CallManager::endCall(const QString &contactId)
{
    if (!mChannels.contains(contactId))
        return;

    mChannels[contactId]->hangup();
    mChannels[contactId]->requestClose();
    mChannels.remove(contactId);
    mContacts.remove(contactId);
}

void CallManager::sendDTMF(const QString &contactId, const QString &key)
{
    if (!mChannels.contains(contactId))
        return;

    foreach(const Tp::CallContentPtr &content, mChannels[contactId]->contents()) {
        if (content->supportsDTMF()) {
            bool ok;
            Tp::DTMFEvent event = (Tp::DTMFEvent)key.toInt(&ok);
            if (!ok) {
                 if (!key.compare("*")) {
                     event = Tp::DTMFEventAsterisk;
                 } else if (!key.compare("#")) {
                     event = Tp::DTMFEventHash;
                 } else {
                     qDebug() << "Tone not recognized. DTMF failed";
                     return;
                 }
            }
            content->startDTMFTone(event);
        }
    }
}

void CallManager::setHold(const QString &contactId, bool hold)
{
    if (!mChannels.contains(contactId))
        return;

    mChannels[contactId]->requestHold(hold);
}

void CallManager::setSpeaker(const QString &contactId, bool speaker)
{
    if (!mChannels.contains(contactId))
        return;

    QDBusInterface androidIf(ANDROID_DBUS_ADDRESS, 
                             ANDROID_TELEPHONY_DBUS_PATH, 
                             ANDROID_TELEPHONY_DBUS_IFACE);
    if (speaker) {
        androidIf.call("turnOnSpeaker", speaker, true);
    } else {
        androidIf.call("restoreSpeakerMode");
    }
}

void CallManager::setMute(const QString &contactId, bool mute)
{
    Tp::ChannelPtr channel = mChannels[contactId];
    if (!channel)
        return;

    // Replace this by a Mute interface method call when it
    // becomes available in telepathy-qt
    QDBusInterface callChannelIf(TP_UFA_DBUS_ADDRESS, 
                                 channel->objectPath(), 
                                 TP_UFA_DBUS_MUTE_FACE);
    callChannelIf.call("RequestMuted", mute);
}

void CallManager::onCallChannelAvailable(Tp::CallChannelPtr channel)
{
    mChannels[channel->targetContact()->id()] = channel;
    connect(channel.data(), SIGNAL(callStateChanged(Tp::CallState)),
                     this, SLOT(onCallStateChanged(Tp::CallState)));

    channel->accept();
    emit callReady(channel->targetContact()->id());
}

void CallManager::onCallStateChanged(Tp::CallState state)
{
    QString contactId;
    Tp::CallChannel *channel =  qobject_cast<Tp::CallChannel*>(sender());
    QMapIterator<QString, Tp::CallChannelPtr> i(mChannels);
    while (i.hasNext()) {
        i.next();
        if (i.value().data() == sender()) {
            contactId = i.key();
        }
    }

    if(!contactId.isNull()) {
        if (state == Tp::CallStateEnded) {
            endCall(contactId);
            emit callEnded(contactId);
        }
    }
}

void CallManager::onContactsAvailable(Tp::PendingOperation *op)
{
    Tp::PendingContacts *pc = qobject_cast<Tp::PendingContacts*>(op);

    if (!pc) {
        qCritical() << "The pending object is not a Tp::PendingContacts";
        return;
    }

    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    // start call to the contacts
    Q_FOREACH(Tp::ContactPtr contact, pc->contacts()) {
        account->ensureAudioCall(contact, QLatin1String("audio"), QDateTime::currentDateTime(), "org.freedesktop.Telepathy.Client.TelephonyApp");

        // hold the ContactPtr to make sure its refcounting stays bigger than 0
        mContacts[contact->id()] = contact;
    }
}
