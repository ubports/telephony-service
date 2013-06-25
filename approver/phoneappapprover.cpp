/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "phoneappapprover.h"
#include "phoneappapproverdbus.h"
#include "phoneapputils.h"
#include "messagingmenu.h"
#include "chatmanager.h"
#include "contactmodel.h"
#include "contactentry.h"
#include "config.h"
#include "ringtone.h"

#include <QDebug>

#include <TelepathyQt/PendingReady>
#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/TextChannel>

namespace C {
#include <libintl.h>
}

#define PHONE_APP_HANDLER TP_QT_IFACE_CLIENT + ".PhoneAppHandler"

PhoneAppApprover::PhoneAppApprover()
: Tp::AbstractClientApprover(channelFilters()),
  mPendingSnapDecision(NULL)
{
    PhoneAppApproverDBus *dbus = new PhoneAppApproverDBus();
    connect(dbus,
            SIGNAL(onMessageSent(const QString&, const QString&)),
            SLOT(onReplyReceived(const QString&, const QString&)));
    connect(dbus,
            SIGNAL(acceptCallRequested()),
            SLOT(onAcceptCallRequested()));
    connect(dbus,
            SIGNAL(rejectCallRequested()),
            SLOT(onRejectCallRequested()));
    dbus->connectToBus();

    connect(MessagingMenu::instance(),
            SIGNAL(replyReceived(QString,QString)),
            SLOT(onReplyReceived(QString,QString)));
    connect(MessagingMenu::instance(), SIGNAL(messageRead(QString,QString)),
            ChatManager::instance(), SLOT(acknowledgeMessage(QString,QString)));

    connect(PhoneAppUtils::instance(),
            SIGNAL(applicationRunningChanged(bool)),
            SLOT(processChannels()));
}

PhoneAppApprover::~PhoneAppApprover()
{
}

Tp::ChannelClassSpecList PhoneAppApprover::channelFilters() const
{
    Tp::ChannelClassSpecList specList;
    specList << Tp::ChannelClassSpec::audioCall();
    specList << Tp::ChannelClassSpec::textChat();

    return specList;
}

Tp::ChannelDispatchOperationPtr PhoneAppApprover::dispatchOperation(Tp::PendingOperation *op)
{
    Tp::ChannelPtr channel = mChannels[op];
    QString accountId = channel->property("accountId").toString();
    Q_FOREACH (Tp::ChannelDispatchOperationPtr dispatchOperation, mDispatchOps) {
        if (dispatchOperation->account()->uniqueIdentifier() == accountId) {
            return dispatchOperation;
        }
    }
    return Tp::ChannelDispatchOperationPtr();
}

void PhoneAppApprover::addDispatchOperation(const Tp::MethodInvocationContextPtr<> &context,
                                        const Tp::ChannelDispatchOperationPtr &dispatchOperation)
{
    bool willHandle = false;

    QList<Tp::ChannelPtr> channels = dispatchOperation->channels();
    Q_FOREACH (Tp::ChannelPtr channel, channels) {

        // Call Channel
        Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(channel);
        if (!callChannel.isNull()) {
            Tp::PendingReady *pr = callChannel->becomeReady(Tp::Features()
                                  << Tp::CallChannel::FeatureCore
                                  << Tp::CallChannel::FeatureCallState);
            mChannels[pr] = callChannel;

            connect(pr, SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onChannelReady(Tp::PendingOperation*)));
            callChannel->setProperty("accountId", QVariant(dispatchOperation->account()->uniqueIdentifier()));
            willHandle = true;
            continue;
        }

        // Text Channel
        Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(channel);
        if (!textChannel.isNull()) {
            // right now we are not using any of the text channel's features in the approver
            // so no need to call becomeReady() on it.
            willHandle = true;
        }
    }

    if (willHandle) {
        mDispatchOps.append(dispatchOperation);
    }

    context->setFinished();

    // check if we need to approve channels already or if we should wait.
    processChannels();
}

class EventData {
public:
    PhoneAppApprover* self;
    Tp::ChannelDispatchOperationPtr dispatchOp;
    Tp::ChannelPtr channel;
};

void action_accept(NotifyNotification* notification,
                   char*               action,
                   gpointer            data)
{
    Q_UNUSED(notification);
    Q_UNUSED(action);

    EventData* eventData = (EventData*) data;
    PhoneAppApprover* approver = (PhoneAppApprover*) eventData->self;
    if (NULL != approver) {
        approver->onApproved((Tp::ChannelDispatchOperationPtr) eventData->dispatchOp);
    }
}

void action_reject(NotifyNotification* notification,
                   char*               action,
                   gpointer            data)
{
    Q_UNUSED(notification);
    Q_UNUSED(action);

    EventData* eventData = (EventData*) data;
    PhoneAppApprover* approver = (PhoneAppApprover*) eventData->self;
    if (NULL != approver) {
        approver->onRejected((Tp::ChannelDispatchOperationPtr) eventData->dispatchOp);
    }
}

void delete_event_data(gpointer data) {
    if (NULL != data)
    delete (EventData*) data;
}

void PhoneAppApprover::onChannelReady(Tp::PendingOperation *op)
{
    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady*>(op);
    if (!pr || !mChannels.contains(pr)) {
        qWarning() << "PendingOperation is not a PendingReady:" << op;
        return;
    }

    Tp::ChannelPtr channel = mChannels[pr];
    Tp::ContactPtr contact = channel->initiatorContact();
    Tp::ChannelDispatchOperationPtr dispatchOp = dispatchOperation(op);

    if (!dispatchOp) {
        return;
    }

    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(channel);
    if (!callChannel) {
        return;
    }

    bool isIncoming = channel->initiatorContact() != dispatchOp->connection()->selfContact();

    if (isIncoming && !callChannel->isRequested() && callChannel->callState() == Tp::CallStateInitialised) {
        callChannel->setRinging();
    } else {
        onApproved(dispatchOp);
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

    // try to find the contact in the ContactModel
    ContactEntry *contactEntry = ContactModel::instance()->contactFromPhoneNumber(contact->id());
    // if the contact is not known, the alias and the number will be the same
    QString title;
    QString icon;
    if (contactEntry) {
        title = contactEntry->displayLabel();
        icon = contactEntry->avatar().toLocalFile();
    } else {
        title = C::gettext("Unknown caller");
    }

    QString body;
    if (!contact->id().isEmpty()) {
        if (contact->id() == "-2") {
            body = QString::fromUtf8(C::gettext("Calling from private number"));
        } else if (contact->id() == "#") {
            body = QString::fromUtf8(C::gettext("Calling from unknown number"));
        } else {
            body = QString::fromUtf8(C::gettext("Calling from %1")).arg(contact->id());
        }
    } else {
        body = C::gettext("Caller number is not available");
    }

    if (icon.isEmpty()) {
        if (!contact->avatarData().fileName.isEmpty()) {
            icon = contact->avatarData().fileName;
        } else {
            icon = phoneAppDirectory() + "/assets/avatar-default@18.png";
        }
    }

    notification = notify_notification_new (title.toStdString().c_str(),
                                            body.toStdString().c_str(),
                                            NULL);
    notify_notification_set_hint_string(notification,
                                        "image_path",
                                        icon.toStdString().c_str());
    notify_notification_set_hint_string(notification,
                                        "x-canonical-snap-decisions",
                                        "true");
    notify_notification_set_hint_string(notification,
                                        "x-canonical-private-button-tint",
                                        "true");

    notify_notification_add_action (notification,
                                    "action_accept",
                                    C::gettext("Accept"),
                                    action_accept,
                                    data,
                                    delete_event_data);
    notify_notification_add_action (notification,
                                    "action_decline_1",
                                    C::gettext("Decline"),
                                    action_reject,
                                    data,
                                    delete_event_data);

    mPendingSnapDecision = notification;

    GError *error = NULL;
    if (!notify_notification_show(notification, &error)) {
        qWarning() << "Failed to show snap decision:" << error->message;
        g_error_free (error);
    }

    // play a ringtone
    Ringtone::instance()->playIncomingCallSound();

    mChannels.remove(pr);
}

void PhoneAppApprover::onApproved(Tp::ChannelDispatchOperationPtr dispatchOp)
{
    closeSnapDecision();

    // forward the channel to the handler
    dispatchOp->handleWith(PHONE_APP_HANDLER);

    // and then launch the phone-app
    PhoneAppUtils::instance()->startPhoneApp();

    mDispatchOps.removeAll(dispatchOp);
}

void PhoneAppApprover::onRejected(Tp::ChannelDispatchOperationPtr dispatchOp)
{
    Tp::PendingOperation *claimop = dispatchOp->claim();
    // assume there is just one channel in the dispatchOp for calls
    mChannels[claimop] = dispatchOp->channels().first();
    connect(claimop, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onClaimFinished(Tp::PendingOperation*)));

    Ringtone::instance()->stopIncomingCallSound();
}

Tp::ChannelDispatchOperationPtr PhoneAppApprover::dispatchOperationForIncomingCall()
{
    Tp::ChannelDispatchOperationPtr callDispatchOp;

    // find the call channel in the dispatch operations
    Q_FOREACH(Tp::ChannelDispatchOperationPtr dispatchOp, mDispatchOps) {
        Q_FOREACH(Tp::ChannelPtr channel, dispatchOp->channels()) {
            Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(channel);
            // FIXME: maybe we need to check the call state too?
            if (!callChannel.isNull()) {
                callDispatchOp = dispatchOp;
                break;
            }
        }

        if (!callDispatchOp.isNull()) {
            break;
        }
    }

    return callDispatchOp;
}

void PhoneAppApprover::processChannels()
{
    Q_FOREACH (Tp::ChannelDispatchOperationPtr dispatchOperation, mDispatchOps) {
        QList<Tp::ChannelPtr> channels = dispatchOperation->channels();
        Q_FOREACH (Tp::ChannelPtr channel, channels) {
            // approve only text channels
            Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(channel);
            if (textChannel.isNull()) {
                continue;
            }

            if (dispatchOperation->possibleHandlers().contains(PHONE_APP_HANDLER)) {
                dispatchOperation->handleWith(PHONE_APP_HANDLER);
                mDispatchOps.removeAll(dispatchOperation);
            }
            // FIXME: this shouldn't happen, but in any case, we need to check what to do when
            // the phone app client is not available
        }
    }
}

void PhoneAppApprover::onClaimFinished(Tp::PendingOperation* op)
{
    if(!op || op->isError()) {
        qDebug() << "onClaimFinished() error";
        // TODO do something
        return;
    }

    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(mChannels[op]);
    if (callChannel) {
        Tp::PendingOperation *hangupop = callChannel->hangup(Tp::CallStateChangeReasonUserRequested, TP_QT_ERROR_REJECTED, QString());
        mChannels[hangupop] = callChannel;
        connect(hangupop, SIGNAL(finished(Tp::PendingOperation*)),
                this, SLOT(onHangupFinished(Tp::PendingOperation*)));
    }
}

void PhoneAppApprover::onHangupFinished(Tp::PendingOperation* op)
{
    if(!op || op->isError()) {
        qDebug() << "onHangupFinished() error";
        // TODO do something
        return;
    }

    // FIXME: we do not call requestClose() here because
    // the channel will be forced to close without emiting the proper
    // stateChanged() signals. This would cause the app
    // not to register call events as it would never receive the
    // "ended" state. Better to check how other connection
    // managers deal with this case.
    mDispatchOps.removeAll(dispatchOperation(op));
    mChannels.remove(op);
}

void PhoneAppApprover::onCallStateChanged(Tp::CallState state)
{
    Tp::CallChannel *channel = qobject_cast<Tp::CallChannel*>(sender());
    if (!channel) {
        return;
    }

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

        closeSnapDecision();
        // add the missed call to the messaging menu
        MessagingMenu::instance()->addCall(channel->targetContact()->id(), QDateTime::currentDateTime());
    } else if (state == Tp::CallStateActive) {
        onApproved(dispatchOperation);
    }
}

void PhoneAppApprover::onReplyReceived(const QString &phoneNumber, const QString &reply)
{
    ChatManager::instance()->sendMessage(phoneNumber, reply);
}

void PhoneAppApprover::closeSnapDecision()
{
    if (mPendingSnapDecision != NULL) {
        notify_notification_close(mPendingSnapDecision, NULL);
        mPendingSnapDecision = NULL;
    }

    Ringtone::instance()->stopIncomingCallSound();
}

void PhoneAppApprover::onAcceptCallRequested()
{
    if (!mPendingSnapDecision) {
        return;
    }

    Tp::ChannelDispatchOperationPtr callDispatchOp = dispatchOperationForIncomingCall();
    if (!callDispatchOp.isNull()) {
        onApproved(callDispatchOp);
    }
}

void PhoneAppApprover::onRejectCallRequested()
{
    if (!mPendingSnapDecision) {
        return;
    }

    Tp::ChannelDispatchOperationPtr callDispatchOp = dispatchOperationForIncomingCall();
    if (!callDispatchOp.isNull()) {
        onRejected(callDispatchOp);
    }
}

