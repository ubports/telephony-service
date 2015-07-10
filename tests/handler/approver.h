/**
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 */

#ifndef APPROVER_H
#define APPROVER_H

#include <QMap>
#include <TelepathyQt/AbstractClientApprover>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/ChannelDispatchOperation>

#define TELEPHONY_SERVICE_HANDLER TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler"
#define TELEPHONY_SERVICE_APPROVER TP_QT_IFACE_CLIENT + ".TelephonyTestApprover"

class Approver : public QObject, public Tp::AbstractClientApprover
{
    Q_OBJECT

public:
    Approver(QObject *parent = 0);
    ~Approver();

    Tp::ChannelClassSpecList channelFilters() const;

    void addDispatchOperation(const Tp::MethodInvocationContextPtr<> &context,
                              const Tp::ChannelDispatchOperationPtr &dispatchOperation);

Q_SIGNALS:
    void newCall();

public Q_SLOTS:
    void acceptCall(bool callAcceptOnChannel = true);
    void rejectCall();

private Q_SLOTS:
    void processChannels();
    void onClaimFinished(Tp::PendingOperation* op);
    void onHangupFinished(Tp::PendingOperation* op);
    void onChannelReady(Tp::PendingOperation *op);

protected:
    Tp::ChannelDispatchOperationPtr dispatchOperation(Tp::PendingOperation *op);

private:
    QList<Tp::ChannelDispatchOperationPtr> mDispatchOps;
    QMap<Tp::PendingOperation*,Tp::ChannelPtr> mChannels;
};

#endif // APPROVER_H
