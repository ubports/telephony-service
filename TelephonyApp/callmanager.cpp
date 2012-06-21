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

void CallManager::startCall(const QString &phoneNumber)
{
    // check if we are already talking to that phone number
    Q_FOREACH(const CallEntry *entry, mCallEntries) {
        if (entry->phoneNumber() == phoneNumber) {
            return;
        }
    }

    // Request the contact to start audio call
    Tp::AccountPtr account = TelepathyHelper::instance()->account();
    connect(account->connection()->contactManager()->contactsForIdentifiers(QStringList() << phoneNumber),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onContactsAvailable(Tp::PendingOperation*)));
}

void CallManager::setSpeaker(bool speaker)
{
    QDBusInterface androidIf(ANDROID_DBUS_ADDRESS,
                             ANDROID_TELEPHONY_DBUS_PATH, 
                             ANDROID_TELEPHONY_DBUS_IFACE);
    if (speaker) {
        androidIf.call("turnOnSpeaker", speaker, true);
    } else {
        androidIf.call("restoreSpeakerMode");
    }
}

QObject *CallManager::foregroundCall() const
{
    // if we have only one call, return it as being always in foreground
    // even if it is held
    if (mCallEntries.count() == 1) {
        return mCallEntries.first();
    }

    Q_FOREACH(CallEntry *entry, mCallEntries) {
        if (!entry->isHeld()) {
            return entry;
        }
    }

    return 0;
}

QObject *CallManager::backgroundCall() const
{
    // if we have only one call, assume there is no call in background
    // even if the foreground call is held
    if (mCallEntries.count() == 1) {
        return 0;
    }

    Q_FOREACH(CallEntry *entry, mCallEntries) {
        if (entry->isHeld()) {
            return entry;
        }
    }

    return 0;
}

bool CallManager::hasCalls() const
{
    return !mCallEntries.isEmpty();
}

void CallManager::onCallChannelAvailable(Tp::CallChannelPtr channel)
{
    CallEntry *entry = new CallEntry(channel, this);
    mCallEntries.append(entry);
    connect(entry,
            SIGNAL(callEnded()),
            SLOT(onCallEnded()));

    // FIXME: check which of those signals we really need to emit here
    emit callReady();
    emit hasCallsChanged();
    emit foregroundCallChanged();
    emit backgroundCallChanged();
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
    // FIXME: handle multiple calls
    CallEntry *entry = qobject_cast<CallEntry*>(sender());
    if (!entry) {
        return;
    }

    emit callEnded();

    // at this point the entry should be removed
    mCallEntries.removeAll(entry);
    delete entry;
    emit hasCallsChanged();
}
