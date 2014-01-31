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
    Q_PROPERTY(QString voicemailNumber
               READ getVoicemailNumber
               NOTIFY voicemailNumberChanged)

public:
    
    static CallManager *instance();
    Q_INVOKABLE void startCall(const QString &phoneNumber);
    Q_INVOKABLE QString getVoicemailNumber();

    CallEntry *foregroundCall() const;
    CallEntry *backgroundCall() const;
    bool hasCalls() const;
    bool hasBackgroundCall() const;

Q_SIGNALS:
    void callEnded(const QString &phoneNumber, bool incoming, const QDateTime &timestamp, const QTime &duratiom, bool missed, bool newEvent);
    void foregroundCallChanged();
    void backgroundCallChanged();
    void hasCallsChanged();
    void hasBackgroundCallChanged();
    void speakerChanged();
    void voicemailNumberChanged();

public Q_SLOTS:
    void onCallChannelAvailable(Tp::CallChannelPtr channel);
    void onChannelObserverUnregistered();
    void onCallEnded();
    void onConnectedChanged();

private:
    explicit CallManager(QObject *parent = 0);
    void refreshProperties();
    void notifyEndedCall(const Tp::CallChannelPtr &channel);
    int activeCallsCount() const;

    QList<CallEntry*> mCallEntries;
    QString mVoicemailNumber;
};

#endif // CALLMANAGER_H
