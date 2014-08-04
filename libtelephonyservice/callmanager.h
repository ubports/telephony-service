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

#ifndef CALLMANAGER_H
#define CALLMANAGER_H

#include <QQmlListProperty>
#include <QtCore/QMap>
#include <QDBusInterface>
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/ReceivedMessage>

class CallEntry;
class TelepathyHelper;

class CallManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *foregroundCall
               READ foregroundCall
               NOTIFY foregroundCallChanged)
    Q_PROPERTY(QObject *backgroundCall
               READ backgroundCall
               NOTIFY backgroundCallChanged)
    Q_PROPERTY(bool hasCalls
               READ hasCalls
               NOTIFY hasCallsChanged)
    Q_PROPERTY(bool hasBackgroundCall
               READ hasBackgroundCall
               NOTIFY hasBackgroundCallChanged)
    Q_PROPERTY(QQmlListProperty<CallEntry> calls
                   READ calls
                   NOTIFY callsChanged)
    Q_PROPERTY(bool callIndicatorVisible
               READ callIndicatorVisible
               WRITE setCallIndicatorVisible
               NOTIFY callIndicatorVisibleChanged)

public:
    static CallManager *instance();
    Q_INVOKABLE void startCall(const QString &phoneNumber, const QString &accountId = QString::null);
    Q_INVOKABLE void mergeCalls(CallEntry *firstCall, CallEntry *secondCall);
    Q_INVOKABLE void splitCall(CallEntry *callEntry);
    Q_INVOKABLE void sendDTMF(const QString &key);

    CallEntry *foregroundCall() const;
    CallEntry *backgroundCall() const;
    QList<CallEntry *> activeCalls() const;
    QQmlListProperty<CallEntry> calls();
    bool hasCalls() const;
    bool hasBackgroundCall() const;

    // conference related members
    QList<CallEntry*> takeCalls(const QList<Tp::ChannelPtr> callChannels);
    void addCalls(const QList<CallEntry*> entries);

    // call indicator related
    bool callIndicatorVisible() const;
    void setCallIndicatorVisible(bool visible);

    // QQmlListProperty helpers
    static int callsCount(QQmlListProperty<CallEntry> *p);
    static CallEntry* callAt(QQmlListProperty<CallEntry> *p, int index);

Q_SIGNALS:
    void callEnded(CallEntry *entry);
    void foregroundCallChanged();
    void backgroundCallChanged();
    void callsChanged();
    void hasCallsChanged();
    void hasBackgroundCallChanged();
    void speakerChanged();
    void voicemailNumberChanged();
    void emergencyNumbersChanged();
    void callIndicatorVisibleChanged(bool visible);

public Q_SLOTS:
    void onCallChannelAvailable(Tp::CallChannelPtr channel);
    void onChannelObserverUnregistered();
    void onCallEnded();
    void onCallIndicatorVisibleChanged(bool visible);

private:
    explicit CallManager(QObject *parent = 0);
    void refreshProperties();
    void setDBusProperty(const QString &name, const QVariant &value);
    void setupCallEntry(CallEntry *entry);

    mutable QList<CallEntry*> mCallEntries;
    bool mNeedsUpdate;
    CallEntry *mConferenceCall;
    bool mCallIndicatorVisible;
};

#endif // CALLMANAGER_H
