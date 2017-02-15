/*
 * Copyright (C) 2012-2014 Canonical, Ltd.
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

#ifndef CALLHANDLER_H
#define CALLHANDLER_H

#include <QtCore/QMap>
#include <QDBusInterface>
#include <TelepathyQt/CallChannel>

class TelepathyHelper;
class CallAgent;

class CallHandler : public QObject
{
    Q_OBJECT

public:
    static CallHandler *instance();
    QVariantMap getCallProperties(const QString &objectPath);
    bool hasCalls() const;

public Q_SLOTS:
    void onCallChannelAvailable(Tp::CallChannelPtr channel);
    void startCall(const QString &targetId, const QString &accountId);
    void hangUpCall(const QString &objectPath);
    void setHold(const QString &objectPath, bool hold);
    void setMuted(const QString &objectPath, bool muted);
    void sendDTMF(const QString &objectPath, const QString &key);

    // conference call related
    void createConferenceCall(const QStringList &objectPaths);
    void mergeCall(const QString &conferenceObjectPath, const QString &callObjectPath);
    void splitCall(const QString &objectPath);

Q_SIGNALS:
    void callPropertiesChanged(const QString &objectPath, const QVariantMap &properties);
    void conferenceCallRequestFinished(bool succeeded);
    void callHoldingFailed(const QString &objectPath);

protected:
    Tp::CallChannelPtr existingCall(const QString &targetId);
    Tp::CallChannelPtr callFromObjectPath(const QString &objectPath);

    void playNextDTMFTone(Tp::CallChannelPtr channel);
    void triggerNextDTMFTone(Tp::CallChannelPtr channel);
    static int toDTMFEvent(const QString &key);
    bool isIncoming(const Tp::CallChannelPtr &channel) const;

    QString applyNumberRewritingRules(const QString &originalNumber, const QVariantMap &properties);

protected Q_SLOTS:
    void onContactsAvailable(Tp::PendingOperation *op);
    void onCallHangupFinished(Tp::PendingOperation *op);
    void onCallChannelInvalidated();
    void onCallStateChanged(Tp::CallState state);

private:
    explicit CallHandler(QObject *parent = 0);

    QMap<QString, Tp::ContactPtr> mContacts;
    QList<Tp::CallChannelPtr> mCallChannels;
    QMap<Tp::CallChannel*,CallAgent*> mCallAgents;
    QMap<Tp::PendingOperation*,Tp::CallChannelPtr> mClosingChannels;
    bool mHangupRequested;
};

#endif // CALLHANDLER_H
