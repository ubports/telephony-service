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
#include "callentry.h"
#include "telepathyhelper.h"

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>

#define ANDROID_DBUS_ADDRESS "com.canonical.Android"
#define ANDROID_TELEPHONY_DBUS_PATH "/com/canonical/android/telephony/Telephony"
#define ANDROID_TELEPHONY_DBUS_IFACE "com.canonical.android.telephony.Telephony"

CallManager::CallManager(QObject *parent)
: QObject(parent)
{
}

bool CallManager::isTalkingToContact(const QString &contactId) const
{
    return mCallEntries.contains(contactId);
}

void CallManager::startCall(const QString &contactId)
{
    if (!mCallEntries.contains(contactId)) {
        // Request the contact to start audio call
        Tp::AccountPtr account = TelepathyHelper::instance()->account();
        connect(account->connection()->contactManager()->contactsForIdentifiers(QStringList() << contactId),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onContactsAvailable(Tp::PendingOperation*)));
    }
}

void CallManager::setSpeaker(const QString &contactId, bool speaker)
{
    Q_UNUSED(contactId)
    QDBusInterface androidIf(ANDROID_DBUS_ADDRESS,
                             ANDROID_TELEPHONY_DBUS_PATH, 
                             ANDROID_TELEPHONY_DBUS_IFACE);
    if (speaker) {
        androidIf.call("turnOnSpeaker", speaker, true);
    } else {
        androidIf.call("restoreSpeakerMode");
    }
}

QObject *CallManager::callEntryForContact(const QString &contactId) const
{
    if (mCallEntries.contains(contactId)) {
        return mCallEntries[contactId];
    }

    return 0;
}

void CallManager::onCallChannelAvailable(Tp::CallChannelPtr channel)
{
    CallEntry *entry = new CallEntry(channel->targetContact()->id(), channel, this);
    mCallEntries[channel->targetContact()->id()] = entry;
    connect(entry,
            SIGNAL(callEnded()),
            SLOT(onCallEnded()));

    emit callReady(entry->contactId());
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

void CallManager::onCallEnded()
{
    CallEntry *entry = qobject_cast<CallEntry*>(sender());
    if (!entry) {
        return;
    }

    emit callEnded(entry->contactId());

    // at this point the entry should be removed
    if (mCallEntries.contains(entry->contactId())) {
        mCallEntries.remove(entry->contactId());
        delete entry;
    }
}
