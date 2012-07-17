/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
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

#include <glib.h>
#include <unistd.h>
#include <libnotify/notify.h>

#include "telephonyappapprover.h"

#include <QDebug>

#include <TelepathyQt/PendingReady>
#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/CallChannel>

TelephonyAppApprover::TelephonyAppApprover()
: Tp::AbstractClientApprover(Tp::ChannelClassSpec::audioCall())
{
}

TelephonyAppApprover::~TelephonyAppApprover()
{
}

Tp::ChannelDispatchOperationPtr TelephonyAppApprover::dispatchOperation(Tp::PendingOperation *op)
{
    Tp::ChannelPtr channel = Tp::ChannelPtr::dynamicCast(mChannels[op]);
    QString accountId = channel->property("accountId").toString();
    foreach (Tp::ChannelDispatchOperationPtr dispatchOperation, mDispatchOps) {
        if (dispatchOperation->account()->uniqueIdentifier() == accountId) {
            return dispatchOperation;
        }
    }
    return Tp::ChannelDispatchOperationPtr();
}

void TelephonyAppApprover::addDispatchOperation(const Tp::MethodInvocationContextPtr<> &context,
                                        const Tp::ChannelDispatchOperationPtr &dispatchOperation)
{
    QList<Tp::ChannelPtr> channels = dispatchOperation->channels();
    foreach (Tp::ChannelPtr channel, channels) {
        Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(channel);
        if (!callChannel.isNull()) {
            dispatchOperation->connection()->becomeReady(Tp::Features()
                                  << Tp::Connection::FeatureSelfContact);
            Tp::PendingReady *pr = callChannel->becomeReady(Tp::Features()
                                  << Tp::CallChannel::FeatureCore
                                  << Tp::CallChannel::FeatureCallState);
            mChannels[pr] = callChannel;

            connect(pr, SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onChannelReady(Tp::PendingOperation*)));
            callChannel->setProperty("accountId", QVariant(dispatchOperation->account()->uniqueIdentifier()));
            mDispatchOps.append(dispatchOperation);
            continue;
        }
    }
    context->setFinished();
}

class EventData {
public:
    TelephonyAppApprover* self;
    Tp::ChannelDispatchOperationPtr dispatchOp;
    Tp::ChannelPtr channel;
    Tp::PendingReady *pr;
};

void action_accept(NotifyNotification* notification,
                   char*               action,
                   gpointer            data)
{
    Q_UNUSED(notification);
    Q_UNUSED(action);

    EventData* eventData = (EventData*) data;
    TelephonyAppApprover* approver = (TelephonyAppApprover*) eventData->self;
    if (NULL != approver) {
        approver->onApproved((Tp::ChannelDispatchOperationPtr) eventData->dispatchOp,
                             (Tp::PendingReady *) eventData->pr);
    }
}

void action_reject(NotifyNotification* notification,
                   char*               action,
                   gpointer            data)
{
    Q_UNUSED(notification);
    Q_UNUSED(action);

    EventData* eventData = (EventData*) data;
    TelephonyAppApprover* approver = (TelephonyAppApprover*) eventData->self;
    if (NULL != approver) {
        approver->onRejected((Tp::ChannelDispatchOperationPtr) eventData->dispatchOp,
                             (Tp::ChannelPtr) eventData->channel);
    }
}

void delete_event_data(gpointer data) {
    if (NULL != data)
    delete (EventData*) data;
}

void TelephonyAppApprover::onChannelReady(Tp::PendingOperation *op)
{
    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady*>(op);
    Tp::ChannelPtr channel = Tp::ChannelPtr::dynamicCast(mChannels[pr]);
    QString accountId = channel->property("accountId").toString();

    Tp::ContactPtr contact = channel->initiatorContact();
    Tp::ChannelDispatchOperationPtr dispatchOp = dispatchOperation(op);
    
    if (!dispatchOp) {
        return;
    }

    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(mChannels[pr]);
    if (!callChannel) {
        return;
    }

    bool isIncoming = channel->initiatorContact()->id() != dispatchOp->connection()->selfContact()->id();

    if (isIncoming) {
        callChannel->setRinging();
    } else {
        onApproved(dispatchOp, NULL);
        return;
    }

    connect(channel.data(),
            SIGNAL(callStateChanged(Tp::CallState)),
            SLOT(onCallStateChanged(Tp::CallState)));

    NotifyNotification* notification;

    /* initial notification */

    EventData* data = new EventData();
    data->self = this;
    data->dispatchOp = dispatchOp;
    data->channel = channel;
    data->pr = pr;

    notification = notify_notification_new ("Incoming call",
                                            QString("Incoming call from %1").arg(contact->id()).toStdString().c_str(),
                                            "");
    notify_notification_set_hint_string(notification,
                                        "x-canonical-snap-decisions",
                                        "true");

    notify_notification_add_action (notification,
                                    "action_accept",
                                    "Accept",
                                    action_accept,
                                    data,
                                    delete_event_data);
    notify_notification_add_action (notification,
                                    "action_decline_1",
                                    "Decline",
                                    action_reject,
                                    data,
                                    delete_event_data);
    GError *error = NULL;
    if (!notify_notification_show(notification, &error)) {
        qWarning() << "Failed to show snap decision:" << error->message;
        g_error_free (error);
    }
}

void TelephonyAppApprover::onApproved(Tp::ChannelDispatchOperationPtr dispatchOp,
                                      Tp::PendingReady *pr)
{
    dispatchOp->handleWith(TP_QT_IFACE_CLIENT + ".TelephonyApp");
    mDispatchOps.removeAll(dispatchOp);
    if (pr) {
        mChannels.remove(pr);
    }
}

void TelephonyAppApprover::onRejected(Tp::ChannelDispatchOperationPtr dispatchOp,
                                      Tp::ChannelPtr channel)
{
    Tp::PendingOperation *claimop = dispatchOp->claim();
    mChannels[claimop] = channel;
    connect(claimop, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onClaimFinished(Tp::PendingOperation*)));
}

void TelephonyAppApprover::onClaimFinished(Tp::PendingOperation* op)
{
    if(!op || op->isError()) {
        qDebug() << "onClaimFinished() error";
        // TODO do something
        return;
    }
    Tp::ChannelPtr channel = Tp::ChannelPtr::dynamicCast(mChannels[op]);
    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(mChannels[op]);
    if (callChannel) {
        Tp::PendingOperation *hangupop = callChannel->hangup(Tp::CallStateChangeReasonUserRequested, TP_QT_ERROR_REJECTED, QString());
        mChannels[hangupop] = callChannel;
        connect(hangupop, SIGNAL(finished(Tp::PendingOperation*)),
                this, SLOT(onHangupFinished(Tp::PendingOperation*)));
        return;
    }

    if (channel) {
        channel->requestClose();
    }
    mDispatchOps.removeAll(dispatchOperation(op));
    mChannels.remove(op);
}

void TelephonyAppApprover::onHangupFinished(Tp::PendingOperation* op)
{
    if(!op || op->isError()) {
        qDebug() << "onHangupFinished() error";
        // TODO do something
        return;
    }
    Tp::ChannelPtr channel = Tp::ChannelPtr::dynamicCast(mChannels[op]);
    if (channel) {
        channel->requestClose();
    }
    mDispatchOps.removeAll(dispatchOperation(op));
    mChannels.remove(op);
}

void TelephonyAppApprover::onCallStateChanged(Tp::CallState state)
{
    Tp::CallChannel *channel = qobject_cast<Tp::CallChannel*>(sender());
    Tp::ChannelDispatchOperationPtr dispatchOperation;
    Q_FOREACH(const Tp::ChannelDispatchOperationPtr &otherDispatchOperation, mDispatchOps) {
        Q_FOREACH(const Tp::ChannelPtr &otherChannel, otherDispatchOperation->channels()) {
            if (otherChannel.data() == channel) {
                dispatchOperation = otherDispatchOperation;
            }
        }
    }

    if(dispatchOperation.isNull()) {
        return;
    }

    if (state == Tp::CallStateEnded) {
        mDispatchOps.removeAll(dispatchOperation);
        // remove all channels and pending operations
        Q_FOREACH(const Tp::ChannelPtr &otherChannel, dispatchOperation->channels()) {
            Tp::PendingOperation* op = mChannels.key(otherChannel);
            if(op) {
                mChannels.remove(op);
            }
        }
        // TODO: close snap decision
    } else if (state == Tp::CallStateActive) {
        onApproved(dispatchOperation, NULL);
    }
}

