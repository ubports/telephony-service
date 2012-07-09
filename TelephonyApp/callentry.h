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

#ifndef CALLENTRY_H
#define CALLENTRY_H

#include <QObject>
#include <QTime>
#include <QContact>
#include <TelepathyQt/CallChannel>

using namespace QtMobility;

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
    // FIXME: handle conference
    Q_PROPERTY(QString phoneNumber
               READ phoneNumber
               NOTIFY phoneNumberChanged)

    Q_PROPERTY(QString contactAlias
               READ contactAlias
               NOTIFY contactAliasChanged)
    Q_PROPERTY(QString contactAvatar
               READ contactAvatar
               NOTIFY contactAvatarChanged)

    Q_PROPERTY(int elapsedTime
               READ elapsedTime
               NOTIFY elapsedTimeChanged)
    Q_PROPERTY(bool active 
               READ isActive
               NOTIFY activeChanged)

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

    QString phoneNumber() const;
    QString contactAlias() const;
    QString contactAvatar() const;

    Q_INVOKABLE void sendDTMF(const QString &key);
    Q_INVOKABLE void endCall();

protected Q_SLOTS:
    void onChannelReady(Tp::PendingOperation *op);
    void onCallStateChanged(Tp::CallState state);
    void onCallFlagsChanged(Tp::CallFlags flags);
    void onCallHangupFinished(Tp::PendingOperation *op);
    void onMutedChanged(uint state);

Q_SIGNALS:
    void callEnded();
    void callActive();
    void activeChanged();
    void heldChanged();
    void mutedChanged();
    void voicemailChanged();
    void phoneNumberChanged();
    void contactAliasChanged();
    void contactAvatarChanged();
    void elapsedTimeChanged();
    
private:
    Tp::CallChannelPtr mChannel;
    QDBusInterface mMuteInterface;
    QContact mContact;
    bool mVoicemail;
    bool mLocalMuteState;
    QTime mElapsedTime;
};

#endif // CALLENTRY_H
