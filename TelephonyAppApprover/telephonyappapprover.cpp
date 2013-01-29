/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "telephonyappapprover.h"
#include "telephonyappapproverdbus.h"
#include "telephonyapputils.h"
#include "messagingmenu.h"
#include "chatmanager.h"
#include "contactmodel.h"
#include "config.h"
#include "ringtone.h"

#include <QDebug>

#include <TelepathyQt/PendingReady>
#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/TextChannel>

#define TELEPHONY_APP_CLIENT TP_QT_IFACE_CLIENT + ".TelephonyApp"
#define TELEPATHY_CALL_IFACE "org.freedesktop.Telepathy.Channel.Type.Call1"


TelephonyAppApprover::TelephonyAppApprover()
: Tp::AbstractClientApprover(channelFilters()),
  mPendingSnapDecision(NULL)
{
    TelephonyAppApproverDBus *dbus = new TelephonyAppApproverDBus();
    connect(dbus, SIGNAL(onMessageSent(const QString&, const QString&)),
                  SLOT(onReplyReceived(const QString&, const QString&)));
    dbus->connectToBus();

    connect(MessagingMenu::instance(),
            SIGNAL(replyReceived(QString,QString)),
            SLOT(onReplyReceived(QString,QString)));
    connect(TelephonyAppUtils::instance(),
            SIGNAL(applicationRunningChanged(bool)),
            SLOT(processChannels()));

    Ringtone::instance()->playIncomingCallSound();
    QTimer::singleShot(3000, Ringtone::instance(), SLOT(stopIncomingCallSound()));
}

TelephonyAppApprover::~TelephonyAppApprover()
{
}

Tp::ChannelClassSpecList TelephonyAppApprover::channelFilters() const
{
    Tp::ChannelClassSpecList specList;
    specList << Tp::ChannelClassSpec::audioCall();
    specList << Tp::ChannelClassSpec::textChat();

    return specList;
}

Tp::ChannelDispatchOperationPtr TelephonyAppApprover::dispatchOperation(Tp::PendingOperation *op)
{
    Tp::ChannelPtr channel = Tp::ChannelPtr::dynamicCast(mChannels[op]);
    QString accountId = channel->property("accountId").toString();
    Q_FOREACH (Tp::ChannelDispatchOperationPtr dispatchOperation, mDispatchOps) {
        if (dispatchOperation->account()->uniqueIdentifier() == accountId) {
            return dispatchOperation;
        }
    }
    return Tp::ChannelDispatchOperationPtr();
}

void TelephonyAppApprover::addDispatchOperation(const Tp::MethodInvocationContextPtr<> &context,
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
        QDBusInterface channelIf(eventData->channel->busName(), eventData->channel->objectPath(), TELEPATHY_CALL_IFACE);
        channelIf.call("turnOnSpeaker", true);
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

    bool isIncoming = channel->initiatorContact() != dispatchOp->connection()->selfContact();

    if (isIncoming && !callChannel->isRequested() && callChannel->callState() == Tp::CallStateInitialised) {
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

    // if the contact is not known, the alias and the number will be the same
    bool unknown = true;
    QString title;
    if (contact->alias() != contact->id()) {
        unknown = false;
        title = contact->alias();
    } else {
        title = "Unknown caller";
    }

    QString body;
    if (!contact->id().isEmpty()) {
        if (contact->id() == "-2") { 
            body = QString("Calling from private number");
        } else if (contact->id() == "#") {
            body = QString("Calling from unknown number");
        } else {
            body = QString("Calling from %1").arg(contact->id());
        }
    } else {
        body = "Caller number is not available";
    }

    QString icon;
    if (!contact->avatarData().fileName.isEmpty()) {
        icon = contact->avatarData().fileName;
    } else {
        icon = telephonyAppDirectory() + "/assets/avatar-default@18.png";
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

    mPendingSnapDecision = notification;

    GError *error = NULL;
    if (!notify_notification_show(notification, &error)) {
        qWarning() << "Failed to show snap decision:" << error->message;
        g_error_free (error);
    }

    // play a ringtone
    Ringtone::instance()->playIncomingCallSound();
}

void TelephonyAppApprover::onApproved(Tp::ChannelDispatchOperationPtr dispatchOp,
                                      Tp::PendingReady *pr)
{
    // launch the telephony-app before dispatching the channel
    TelephonyAppUtils::instance()->startTelephonyApp();

    dispatchOp->handleWith(TELEPHONY_APP_CLIENT);
    mDispatchOps.removeAll(dispatchOp);
    if (pr) {
        mChannels.remove(pr);
    }
    closeSnapDecision();
}

void TelephonyAppApprover::onRejected(Tp::ChannelDispatchOperationPtr dispatchOp,
                                      Tp::ChannelPtr channel)
{
    Tp::PendingOperation *claimop = dispatchOp->claim();
    mChannels[claimop] = channel;
    connect(claimop, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onClaimFinished(Tp::PendingOperation*)));
}

void TelephonyAppApprover::processChannels()
{
    // if the telephony app is not running, do not approve text channels
    if (!TelephonyAppUtils::instance()->isApplicationRunning()) {
        return;
    }

    Q_FOREACH (Tp::ChannelDispatchOperationPtr dispatchOperation, mDispatchOps) {
        QList<Tp::ChannelPtr> channels = dispatchOperation->channels();
        Q_FOREACH (Tp::ChannelPtr channel, channels) {
            // approve only text channels
            Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(channel);
            if (textChannel.isNull()) {
                continue;
            }

            if (dispatchOperation->possibleHandlers().contains(TELEPHONY_APP_CLIENT)) {
                dispatchOperation->handleWith(TELEPHONY_APP_CLIENT);
                mDispatchOps.removeAll(dispatchOperation);
            }
            // FIXME: this shouldn't happen, but in any case, we need to check what to do when
            // the telephony app client is not available
        }
    }
}

void TelephonyAppApprover::onClaimFinished(Tp::PendingOperation* op)
{
    if(!op || op->isError()) {
        qDebug() << "onClaimFinished() error";
        // TODO do something
        return;
    }
    Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(mChannels[op]);
    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(mChannels[op]);
    if (callChannel) {
        Tp::PendingOperation *hangupop = callChannel->hangup(Tp::CallStateChangeReasonUserRequested, TP_QT_ERROR_REJECTED, QString());
        mChannels[hangupop] = callChannel;
        connect(hangupop, SIGNAL(finished(Tp::PendingOperation*)),
                this, SLOT(onHangupFinished(Tp::PendingOperation*)));
    }

    if (textChannel) {
        QString phoneNumber = textChannel->targetContact()->id();
        QString message = mPendingMessages.take(phoneNumber);

        ChatManager *chatManager = ChatManager::instance();
        chatManager->onTextChannelAvailable(textChannel);
        chatManager->sendMessage(phoneNumber, message);
        chatManager->acknowledgeMessages(phoneNumber);
        chatManager->endChat(phoneNumber);
    }
}

void TelephonyAppApprover::onHangupFinished(Tp::PendingOperation* op)
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

        closeSnapDecision();
        // add the missed call to the messaging menu
        MessagingMenu::instance()->addCall(channel->targetContact()->id(), QDateTime::currentDateTime());
    } else if (state == Tp::CallStateActive) {
        onApproved(dispatchOperation, NULL);
    }
}

void TelephonyAppApprover::onReplyReceived(const QString &phoneNumber, const QString &reply)
{
    // if the app is running, just send using it
    if (TelephonyAppUtils::instance()->isApplicationRunning()) {
        ChatManager::instance()->sendMessage(phoneNumber, reply);
        return;
    }

    // if it is not, find the channel, claim it, mark messages as read and send the reply.
    mPendingMessages[phoneNumber] = reply;
    Tp::ChannelDispatchOperationPtr dispatchOperation;
    Tp::TextChannelPtr textChannel;
    Q_FOREACH(const Tp::ChannelDispatchOperationPtr &otherDispatchOperation, mDispatchOps) {
        Q_FOREACH(const Tp::ChannelPtr &channel, otherDispatchOperation->channels()) {
            Tp::TextChannelPtr otherTextChannel = Tp::TextChannelPtr::dynamicCast(channel);
            if (otherTextChannel && ContactModel::comparePhoneNumbers(channel->targetContact()->id(), phoneNumber)) {
                dispatchOperation = otherDispatchOperation;
                textChannel = otherTextChannel;
            }
        }
    }

    if (!dispatchOperation.isNull()) {
        Tp::PendingOperation *claimop = dispatchOperation->claim();
        mChannels[claimop] = textChannel;
        connect(claimop, SIGNAL(finished(Tp::PendingOperation*)),
                this, SLOT(onClaimFinished(Tp::PendingOperation*)));
    } else {
        // if there is no dispatch operation, just send using the chatmanager
        ChatManager::instance()->sendMessage(phoneNumber, reply);
    }
}

void TelephonyAppApprover::closeSnapDecision()
{
    if (mPendingSnapDecision != NULL) {
        notify_notification_close(mPendingSnapDecision, NULL);
        mPendingSnapDecision = NULL;
        Ringtone::instance()->stopIncomingCallSound();
    }
}

