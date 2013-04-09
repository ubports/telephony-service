/*
 * Copyright (C) 2012 Canonical, Ltd.
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

#include "callmanager.h"
#include "callentry.h"
#include "contactmodel.h"
#include "telepathyhelper.h"

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>
#include <QDBusInterface>

#define CANONICAL_IFACE_TELEPHONY "com.canonical.Telephony"

typedef QMap<QString, QVariant> dbusQMap;
Q_DECLARE_METATYPE(dbusQMap)

CallManager *CallManager::instance()
{
    static CallManager *self = new CallManager();
    return self;
}

CallManager::CallManager(QObject *parent)
: QObject(parent)
{
    connect(TelepathyHelper::instance(), SIGNAL(connectedChanged()), SLOT(onConnectedChanged()));
}

void CallManager::startCall(const QString &phoneNumber)
{
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->call("StartCall", phoneNumber);
}

void CallManager::onConnectedChanged()
{
    if (!TelepathyHelper::instance()->connected()) {
        mVoicemailNumber = QString();
        Q_EMIT voicemailNumberChanged();
        return;
    }

    Tp::ConnectionPtr conn(TelepathyHelper::instance()->account()->connection());
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    QDBusInterface connIface(busName, objectPath, CANONICAL_IFACE_TELEPHONY);
    QDBusReply<QString> replyNumber = connIface.call("VoicemailNumber");
    if (replyNumber.isValid()) {
        mVoicemailNumber = replyNumber.value();
        Q_EMIT voicemailNumberChanged();
    }
}

QObject *CallManager::foregroundCall() const
{
    CallEntry *call = 0;

    // if we have only one call, return it as being always in foreground
    // even if it is held
    if (mCallEntries.count() == 1) {
        call = mCallEntries.first();
    }

    Q_FOREACH(CallEntry *entry, mCallEntries) {
        if (entry->isActive() && !entry->isHeld()) {
            call = entry;
        }
    }

    if (call && (call->isActive() || call->isHeld() || !call->incoming())) {
        return call;
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

bool CallManager::hasBackgroundCall() const
{
    return mCallEntries.count() > 1;
}

void CallManager::onCallChannelAvailable(Tp::CallChannelPtr channel)
{
    CallEntry *entry = new CallEntry(channel, this);
    if (entry->phoneNumber() == getVoicemailNumber()) {
        entry->setVoicemail(true);
    }

    if (channel->isRequested()) {
        entry->setSpeaker(true);
    }

    mCallEntries.append(entry);
    connect(entry,
            SIGNAL(callEnded()),
            SLOT(onCallEnded()));
    connect(entry,
            SIGNAL(heldChanged()),
            SIGNAL(foregroundCallChanged()));
    connect(entry,
            SIGNAL(activeChanged()),
            SIGNAL(foregroundCallChanged()));
    connect(entry,
            SIGNAL(heldChanged()),
            SIGNAL(backgroundCallChanged()));

    // FIXME: check which of those signals we really need to emit here
    Q_EMIT hasCallsChanged();
    Q_EMIT hasBackgroundCallChanged();
    Q_EMIT foregroundCallChanged();
    Q_EMIT backgroundCallChanged();
}

void CallManager::onCallEnded()
{
    // FIXME: handle multiple calls
    CallEntry *entry = qobject_cast<CallEntry*>(sender());
    if (!entry) {
        return;
    }

    // at this point the entry should be removed
    mCallEntries.removeAll(entry);

    notifyEndedCall(entry->channel());
    Q_EMIT hasCallsChanged();
    Q_EMIT hasBackgroundCallChanged();
    Q_EMIT foregroundCallChanged();
    Q_EMIT backgroundCallChanged();
    entry->deleteLater();
}

QString CallManager::getVoicemailNumber()
{
    return mVoicemailNumber;
}


void CallManager::notifyEndedCall(const Tp::CallChannelPtr &channel)
{
    Tp::Contacts contacts = channel->remoteMembers();
    if (contacts.isEmpty()) {
        qWarning() << "Call channel had no remote contacts:" << channel;
        return;
    }

    QString phoneNumber;
    // FIXME: handle conference call
    Q_FOREACH(const Tp::ContactPtr &contact, contacts) {
        phoneNumber = contact->id();
        break;
    }

    // fill the call info
    QDateTime timestamp = channel->property("timestamp").toDateTime();
    bool incoming = channel->initiatorContact() != TelepathyHelper::instance()->account()->connection()->selfContact();
    QTime duration(0, 0, 0);
    bool missed = incoming && channel->callStateReason().reason == Tp::CallStateChangeReasonNoAnswer;

    if (!missed) {
        QDateTime activeTime = channel->property("activeTimestamp").toDateTime();
        duration = duration.addSecs(activeTime.secsTo(QDateTime::currentDateTime()));
    }

    // and finally add the entry
    // just mark it as new if it is missed
    Q_EMIT callEnded(phoneNumber, incoming, timestamp, duration, missed, missed);
}
