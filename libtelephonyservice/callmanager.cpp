/*
 * Copyright (C) 2012 Canonical, Ltd.
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

#include "callmanager.h"
#include "callentry.h"
#include "telepathyhelper.h"

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>
#include <QDBusInterface>

typedef QMap<QString, QVariant> dbusQMap;
Q_DECLARE_METATYPE(dbusQMap)

CallManager *CallManager::instance()
{
    static CallManager *self = new CallManager();
    return self;
}

CallManager::CallManager(QObject *parent)
: QObject(parent), mConferenceCall(0)
{
    connect(TelepathyHelper::instance(), SIGNAL(connectedChanged()), SLOT(onConnectedChanged()));
    connect(TelepathyHelper::instance(), SIGNAL(channelObserverUnregistered()), SLOT(onChannelObserverUnregistered()));
    connect(this, SIGNAL(hasCallsChanged()), SIGNAL(callsChanged()));
}

QList<CallEntry *> CallManager::takeCalls(const QList<Tp::ChannelPtr> callChannels)
{
    qDebug() << __PRETTY_FUNCTION__;
    QList<CallEntry*> entries;

    // run through the current calls and check which ones we find
    Q_FOREACH(CallEntry *entry, mCallEntries) {
        if (callChannels.contains(entry->channel())) {
            mCallEntries.removeAll(entry);
            entries << entry;
            entry->disconnect(this);
        }
    }

    // FIXME: check which of those signals we really need to emit here
    Q_EMIT hasCallsChanged();
    Q_EMIT hasBackgroundCallChanged();
    Q_EMIT foregroundCallChanged();
    Q_EMIT backgroundCallChanged();

    return entries;
}

void CallManager::addCalls(const QList<CallEntry *> entries)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_FOREACH (CallEntry *entry, entries) {
        mCallEntries << entry;
        setupCallEntry(entry);
    }

    // FIXME: check which of those signals we really need to emit here
    Q_EMIT hasCallsChanged();
    Q_EMIT hasBackgroundCallChanged();
    Q_EMIT foregroundCallChanged();
    Q_EMIT backgroundCallChanged();
}

void CallManager::setupCallEntry(CallEntry *entry)
{
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
}

void CallManager::onChannelObserverUnregistered()
{
    mConferenceCall = 0;
    mCallEntries.clear();
    Q_EMIT hasCallsChanged();
    Q_EMIT hasBackgroundCallChanged();
    Q_EMIT foregroundCallChanged();
    Q_EMIT backgroundCallChanged();
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
    QDBusInterface connIface(busName, objectPath, CANONICAL_TELEPHONY_VOICEMAIL_IFACE);
    QDBusReply<QString> replyNumber = connIface.call("VoicemailNumber");
    if (replyNumber.isValid()) {
        mVoicemailNumber = replyNumber.value();
        Q_EMIT voicemailNumberChanged();
    }
}

CallEntry *CallManager::foregroundCall() const
{
    CallEntry *call = 0;

    // if we have only one call, return it as being always in foreground
    // even if it is held
    QList<CallEntry*> calls = activeCalls();
    if (calls.count() == 1) {
        call = calls.first();
    } else {
        Q_FOREACH(CallEntry *entry, calls) {
            if (entry->isActive() && !entry->isHeld()) {
                call = entry;
                break;
            }
        }
    }

    return call;
}

CallEntry *CallManager::backgroundCall() const
{
    QList<CallEntry*> calls = activeCalls();
    // if we have only one call, assume there is no call in background
    // even if the foreground call is held
    if (calls.count() == 1) {
        return 0;
    }

    Q_FOREACH(CallEntry *entry, calls) {
        if (entry->isHeld()) {
            return entry;
        }
    }

    return 0;
}

QList<CallEntry *> CallManager::activeCalls() const
{
    QList<CallEntry*> calls;
    if (mConferenceCall) {
        calls << mConferenceCall;
    }

    Q_FOREACH(CallEntry *entry, mCallEntries) {
        if (entry->isActive() || entry->dialing()) {
            calls << entry;
        }
    }

    return calls;
}

QQmlListProperty<CallEntry> CallManager::calls()
{
    return QQmlListProperty<CallEntry>(this, 0, callsCount, callAt);
}

bool CallManager::hasCalls() const
{
    return activeCalls().count() > 0;
}

bool CallManager::hasBackgroundCall() const
{
    return activeCalls().count() > 1;
}

int CallManager::callsCount(QQmlListProperty<CallEntry> *p)
{
    return CallManager::instance()->activeCalls().count();
}

CallEntry *CallManager::callAt(QQmlListProperty<CallEntry> *p, int index)
{
    return CallManager::instance()->activeCalls()[index];
}

void CallManager::onCallChannelAvailable(Tp::CallChannelPtr channel)
{
    CallEntry *entry = new CallEntry(channel, this);
    if (entry->phoneNumber() == getVoicemailNumber()) {
        entry->setVoicemail(true);
    }

    if (entry->isConference()) {
        // assume there can be only one conference call at any time for now
        mConferenceCall = entry;

        // check if any of the existing channels belong to the conference
        // if they do, move them to the conference
        QList<Tp::ChannelPtr> channels = channel->conferenceChannels();
        Q_FOREACH(CallEntry *existingCall, mCallEntries) {
            if (channels.contains(existingCall->channel())) {
                mCallEntries.removeAll(existingCall);
                mConferenceCall->addCall(existingCall);
            }
        }
    } else if (mConferenceCall && mConferenceCall->channel()->conferenceChannels().contains(channel)){
        // if the call channel belongs to the conference, don't add it here, move it to the conference itself
        mConferenceCall->addCall(entry);
    } else {
        mCallEntries.append(entry);
    }

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
    connect(entry,
            SIGNAL(activeChanged()),
            SIGNAL(hasBackgroundCallChanged()));

    // FIXME: check which of those signals we really need to emit here
    Q_EMIT hasCallsChanged();
    Q_EMIT hasBackgroundCallChanged();
    Q_EMIT foregroundCallChanged();
    Q_EMIT backgroundCallChanged();
}

void CallManager::onCallEnded()
{
    qDebug() << __PRETTY_FUNCTION__;
    // FIXME: handle multiple calls
    CallEntry *entry = qobject_cast<CallEntry*>(sender());
    if (!entry) {
        return;
    }

    // at this point the entry should be removed
    if (entry == mConferenceCall) {
        mConferenceCall = 0;
    } else {
        mCallEntries.removeAll(entry);
    }

    Q_EMIT callEnded(entry);
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

void CallManager::mergeCalls(CallEntry *firstCall, CallEntry *secondCall)
{
    QDBusInterface *handlerInterface = TelepathyHelper::instance()->handlerInterface();
    // if there is already a conference call, just merge the remaining channels
    // in the existing conference
    if (firstCall->isConference() || secondCall->isConference()) {
        CallEntry *conferenceCall = firstCall->isConference() ? firstCall : secondCall;
        CallEntry *otherCall = firstCall->isConference() ? secondCall : firstCall;
        handlerInterface->call("MergeCall", conferenceCall->channel()->objectPath(), otherCall->channel()->objectPath());
    } else {
        handlerInterface->call("CreateConferenceCall", QStringList() << firstCall->channel()->objectPath()
                                                                     << secondCall->channel()->objectPath());
    }
}

void CallManager::splitCall(CallEntry *callEntry)
{
    QDBusInterface *handlerInterface = TelepathyHelper::instance()->handlerInterface();
    handlerInterface->call("SplitCall", callEntry->channel()->objectPath());
}
