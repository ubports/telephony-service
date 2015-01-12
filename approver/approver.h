/*
 * Copyright (C) 2012-2014 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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

#ifndef APPROVER_H
#define APPROVER_H

#include <glib.h>
#include <unistd.h>
#include <libnotify/notify.h>

#include <QContact>
#include <QMap>
#include <TelepathyQt/AbstractClientApprover>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/ChannelDispatchOperation>
#include <QFeedbackHapticsEffect>

QTCONTACTS_USE_NAMESPACE

class Approver : public QObject, public Tp::AbstractClientApprover
{
    Q_OBJECT

public:
    Approver();
    ~Approver();

    Tp::ChannelClassSpecList channelFilters() const;

    void addDispatchOperation(const Tp::MethodInvocationContextPtr<> &context,
                              const Tp::ChannelDispatchOperationPtr &dispatchOperation);
    Tp::ChannelDispatchOperationPtr dispatchOperation(Tp::PendingOperation *op);
    void onApproved(Tp::ChannelDispatchOperationPtr dispatchOp);
    void onHangUpAndApproved(Tp::ChannelDispatchOperationPtr dispatchOp);
    void onRejected(Tp::ChannelDispatchOperationPtr dispatchOp);
    void onRejectMessage(Tp::ChannelDispatchOperationPtr dispatchOp, const char *action);
    bool showSnapDecision(const Tp::ChannelDispatchOperationPtr dispatchOperation,
                          const Tp::ChannelPtr channel,
                          const QContact &contact = QContact());

protected:
    Tp::ChannelDispatchOperationPtr dispatchOperationForIncomingCall();

private Q_SLOTS:
    void processChannels();
    void onChannelReady(Tp::PendingOperation *op);
    void onClaimFinished(Tp::PendingOperation* op);
    void onHangupFinished(Tp::PendingOperation* op);
    void onCallStateChanged(Tp::CallState state);
    void closeSnapDecision();
    void onHangupAndAcceptCallRequested();
    void onAcceptCallRequested();
    void onRejectCallRequested();
    void onHandleMediaKeyRequested(bool doubleClick);
    void updateNotification(const QtContacts::QContact &contact);

private:
    QList<Tp::ChannelDispatchOperationPtr> mDispatchOps;
    QMap<Tp::PendingOperation*,Tp::ChannelPtr> mChannels;
    NotifyNotification* mPendingSnapDecision;
    QString mDefaultTitle;
    QString mDefaultIcon;
    QString mCachedBody;
    QFeedbackHapticsEffect mVibrateEffect;
    QTimer mVibrateTimer;
    QMap<QString,QString> mRejectActions;
    bool mFirstTime;
};

#endif // APPROVER_H
