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

#ifndef CALLENTRY_H
#define CALLENTRY_H

#include <QQmlListProperty>
#include <QObject>
#include <QTime>
#include <TelepathyQt/CallChannel>

class CallEntry : public QObject
{
    Q_OBJECT
    Q_PROPERTY (bool held
                READ isHeld
                WRITE setHold
                NOTIFY heldChanged)
    Q_PROPERTY(bool muted
               READ isMuted
               WRITE setMute
               NOTIFY mutedChanged)
    Q_PROPERTY(bool voicemail
               READ isVoicemail
               WRITE setVoicemail
               NOTIFY voicemailChanged)

    // this property is only filled for 1-1 calls
    Q_PROPERTY(QString phoneNumber
               READ phoneNumber
               NOTIFY phoneNumberChanged)

    // this property is only filled for conference calls
    Q_PROPERTY(QQmlListProperty<CallEntry> calls
                       READ calls
                       NOTIFY callsChanged)
    Q_PROPERTY(bool isConference
               READ isConference
               NOTIFY isConferenceChanged)

    Q_PROPERTY(int elapsedTime
               READ elapsedTime
               NOTIFY elapsedTimeChanged)
    Q_PROPERTY(bool active 
               READ isActive
               NOTIFY activeChanged)
    Q_PROPERTY(bool dialing
               READ dialing
               NOTIFY dialingChanged)
    Q_PROPERTY(bool incoming
               READ incoming
               NOTIFY incomingChanged)
    Q_PROPERTY(bool ringing
               READ ringing
               NOTIFY ringingChanged)
    Q_PROPERTY(bool speaker
               READ isSpeakerOn
               WRITE setSpeaker
               NOTIFY speakerChanged)
    Q_PROPERTY(QString dtmfString
               READ dtmfString
               NOTIFY dtmfStringChanged)

public:
    explicit CallEntry(const Tp::CallChannelPtr &channel, QObject *parent = 0);
    void timerEvent(QTimerEvent *event);

    bool isHeld() const;
    void setHold(bool hold);

    bool isMuted() const;
    void setMute(bool value);

    bool isVoicemail() const;
    void setVoicemail(bool voicemail);

    int elapsedTime() const;
    bool isActive() const;

    bool isSpeakerOn();
    Q_INVOKABLE void setSpeaker(bool speaker);

    bool dialing() const;
    bool incoming() const;
    bool ringing() const;
    QString phoneNumber() const;
    QQmlListProperty<CallEntry> calls();
    bool isConference() const;
    QString dtmfString() const;

    Q_INVOKABLE void sendDTMF(const QString &key);
    Q_INVOKABLE void endCall();

    Tp::CallChannelPtr channel() const;

    // QQmlListProperty helpers
    static int callsCount(QQmlListProperty<CallEntry> *p);
    static CallEntry* callAt(QQmlListProperty<CallEntry> *p, int index);

protected Q_SLOTS:
    void onCallStateChanged(Tp::CallState state);
    void onCallFlagsChanged(Tp::CallFlags flags);
    void onMutedChanged(uint state);
    void onSpeakerChanged(bool active);
    void onCallPropertiesChanged(const QString &objectPath, const QVariantMap &properties);

    // conference related stuff
    void onConferenceChannelMerged(const Tp::ChannelPtr &channel);
    void onConferenceChannelRemoved(const Tp::ChannelPtr &channel, const Tp::Channel::GroupMemberChangeDetails &details);
    void onInternalCallEnded();

protected:
    void setupCallChannel();
    void updateChannelProperties(const QVariantMap &properties = QVariantMap());

Q_SIGNALS:
    void callEnded();
    void callActive();
    void activeChanged();
    void heldChanged();
    void mutedChanged();
    void voicemailChanged();
    void phoneNumberChanged();
    void callsChanged();
    void isConferenceChanged();
    void dtmfStringChanged();
    void dialingChanged();
    void incomingChanged();
    void ringingChanged();
    void elapsedTimeChanged();
    void speakerChanged();
    
private:
    void refreshProperties();

    Tp::CallChannelPtr mChannel;
    QDBusInterface mMuteInterface;
    QDBusInterface mSpeakerInterface;
    QMap<QString, QVariant> mProperties;
    bool mVoicemail;
    bool mLocalMuteState;
    QDateTime mActiveTimestamp;
    bool mHasSpeakerProperty;
    bool mSpeakerMode;
    QList<CallEntry*> mCalls;
};

#endif // CALLENTRY_H
