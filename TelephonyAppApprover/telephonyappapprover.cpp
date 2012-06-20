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
#include <QMessageBox>

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
            Tp::PendingReady *pr = callChannel->becomeReady();
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

void action_accept(NotifyNotification* notification,
                   char*               action,
                   gpointer            data)
{
    qDebug() << "GOT IT";
}

void TelephonyAppApprover::onChannelReady(Tp::PendingOperation *op)
{
    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady*>(op);
    Tp::ChannelPtr channel = Tp::ChannelPtr::dynamicCast(mChannels[pr]);
    QString accountId = channel->property("accountId").toString();

    if (channel->isRequested()) {
        return;
    }

    Tp::ContactPtr contact = channel->initiatorContact();
    Tp::ChannelDispatchOperationPtr dispatchOp = dispatchOperation(op);
    
    if (!dispatchOp) {
        return;
    }

    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(mChannels[pr]);
    if (callChannel) {
        callChannel->setRinging();
    }

    NotifyNotification* notification;
    gboolean            success;
    GError*             error = NULL;

    /* initial notification */

    qDebug() << "FUCKCKCKC";

    notification = notify_notification_new ("HELLO", "dobobobob", "");
    qDebug() << "444444";

    notify_notification_add_action (notification,
                                    "action_accept",
                                    "Accept",
                                    action_accept,
                                    NULL,
                                    NULL);

    notify_notification_add_action (notification,
                                    "action_decline_1",
                                    "Decline",
                                    action_accept,
                                    NULL,
                                    NULL);
    notify_notification_show(notification, &error);
    qDebug() << "FUCKCKCKC";

    int ret = QMessageBox::question(NULL, "HELLO",
                    QString("Incoming call from %1\nAnswer?").arg(contact->id()),
                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        dispatchOp->handleWith(TP_QT_IFACE_CLIENT + ".TelephonyApp");
        mDispatchOps.removeAll(dispatchOp);
        mChannels.remove(pr);
        return;
    } else {
        Tp::PendingOperation *claimop = dispatchOp->claim();
        mChannels[claimop] = channel;
        connect(claimop, SIGNAL(finished(Tp::PendingOperation*)),
                this, SLOT(onClaimFinished(Tp::PendingOperation*)));
    }
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
