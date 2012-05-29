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

#include "telephonyappapprover.h"
#include <QMessageBox>

#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/ChannelClassSpec>
#include <TelepathyQt4/ClientRegistrar>
#include <TelepathyQt4/ChannelDispatchOperation>
#include <TelepathyQt4/CallChannel>

TelephonyAppApprover::TelephonyAppApprover()
: Tp::AbstractClientApprover(Tp::ChannelClassSpec::audioCall())
{
}

TelephonyAppApprover::~TelephonyAppApprover()
{
}

void TelephonyAppApprover::addDispatchOperation(const Tp::MethodInvocationContextPtr<> &context,
                                        const Tp::ChannelDispatchOperationPtr &dispatchOperation)
{
    QList<Tp::ChannelPtr> channels = dispatchOperation->channels();
    foreach (Tp::ChannelPtr channel, channels) {
        Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(channel);
        if (!callChannel.isNull()) {
            mChannels[callChannel->becomeReady()] = callChannel;
            callChannel->setProperty("accountId", QVariant(dispatchOperation->account()->uniqueIdentifier()));
            connect(callChannel->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onChannelReady(Tp::PendingOperation*)));
            mDispatchOps.append(dispatchOperation);
            continue;
        }
    }
    context->setFinished();
}

void TelephonyAppApprover::onChannelReady(Tp::PendingOperation *op)
{
    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady*>(op);
    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(mChannels[pr]);
    QString accountId = callChannel->property("accountId").toString();
    Tp::ContactPtr contact = callChannel->initiatorContact();
    foreach (Tp::ChannelDispatchOperationPtr dispatchOperation, mDispatchOps) {
        if (dispatchOperation->account()->uniqueIdentifier() != accountId || callChannel->isRequested()) {
            continue;
        }
        int ret = QMessageBox::question(NULL, "Incoming call",
                        QString("Incoming call from %1\nAnswer?").arg(contact->id()),
                        QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            dispatchOperation->handleWith(TP_QT_IFACE_CLIENT + ".TelephonyApp");
        }
        mDispatchOps.removeAll(dispatchOperation);
    }
    mChannels.remove(pr);
}
