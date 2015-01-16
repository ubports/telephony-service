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

#include "approver.h"
#include "approverdbus.h"
#include "applicationutils.h"
#include "callnotification.h"
#include "chatmanager.h"
#include "config.h"
#include "contactutils.h"
#include "greetercontacts.h"
#include "ringtone.h"
#include "callmanager.h"
#include "callentry.h"
#include "tonegenerator.h"
#include "telepathyhelper.h"

#include <QContactAvatar>
#include <QContactDisplayLabel>
#include <QContactFetchRequest>
#include <QContactPhoneNumber>
#include <QDebug>
#include <QFeedbackHapticsEffect>

#include <TelepathyQt/PendingReady>
#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/TextChannel>

namespace C {
#include <libintl.h>
}

#define TELEPHONY_SERVICE_HANDLER TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler"

Approver::Approver()
: Tp::AbstractClientApprover(channelFilters()),
  mPendingSnapDecision(NULL),
  mSettleTimer(new QTimer(this))
{
    mDefaultTitle = C::gettext("Unknown caller");
    mDefaultIcon = QUrl(telephonyServiceDir() + "assets/avatar-default@18.png").toEncoded();

    ApproverDBus *dbus = new ApproverDBus(this);
    connect(dbus,
            SIGNAL(acceptCallRequested()),
            SLOT(onAcceptCallRequested()));
    connect(dbus,
            SIGNAL(rejectCallRequested()),
            SLOT(onRejectCallRequested()));

    dbus->connectToBus();

    if (GreeterContacts::isGreeterMode()) {
        connect(GreeterContacts::instance(), SIGNAL(contactUpdated(QtContacts::QContact)),
                this, SLOT(updateNotification(QtContacts::QContact)));
    }
    // WORKAROUND: we need to use a timer as the qtubuntu sensors backend does not support setPeriod()
    mVibrateTimer.setInterval(4000);
    connect(&mVibrateTimer, SIGNAL(timeout()), &mVibrateEffect, SLOT(start()));

    mRejectActions["rejectMessage1"] = C::gettext("I'm busy at the moment. I'll call later.");
    mRejectActions["rejectMessage2"] = C::gettext("I'm running late, on my way now.");
    mRejectActions["rejectMessage3"] = C::gettext("Please call me back later.");

    mSettleTimer->setInterval(500);
    mSettleTimer->setSingleShot(true);
    connect(mSettleTimer, SIGNAL(timeout()), this, SLOT(onSettleTimerTimeout()));
    mSettleTimer->start();
}

void Approver::onSettleTimerTimeout()
{
    mSettleTimer->deleteLater();
    mSettleTimer = NULL;
}

Approver::~Approver()
{
}

Tp::ChannelClassSpecList Approver::channelFilters() const
{
    Tp::ChannelClassSpecList specList;
    specList << Tp::ChannelClassSpec::audioCall();
    specList << Tp::ChannelClassSpec::textChat();

    return specList;
}

Tp::ChannelDispatchOperationPtr Approver::dispatchOperation(Tp::PendingOperation *op)
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

void Approver::addDispatchOperation(const Tp::MethodInvocationContextPtr<> &context,
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
    Approver* self;
    Tp::ChannelDispatchOperationPtr dispatchOp;
    Tp::ChannelPtr channel;
};

void action_accept(NotifyNotification *notification, char *action, gpointer data)
{
    Q_UNUSED(notification);
    Q_UNUSED(action);

    EventData* eventData = (EventData*) data;
    Approver* approver = (Approver*) eventData->self;
    if (NULL != approver) {
        approver->onApproved((Tp::ChannelDispatchOperationPtr) eventData->dispatchOp);
    }
}

void action_hangup_and_accept(NotifyNotification *notification, char *action, gpointer data)
{
    Q_UNUSED(notification);
    Q_UNUSED(action);

    EventData *eventData = (EventData*) data;
    Approver *approver = (Approver*) eventData->self;
    if (approver != NULL) {
        approver->onHangUpAndApproved((Tp::ChannelDispatchOperationPtr) eventData->dispatchOp);
    }
}

void action_reject(NotifyNotification *notification, char *action, gpointer data)
{
    Q_UNUSED(notification);
    Q_UNUSED(action);

    EventData* eventData = (EventData*) data;
    Approver* approver = (Approver*) eventData->self;
    if (NULL != approver) {
        approver->onRejected((Tp::ChannelDispatchOperationPtr) eventData->dispatchOp);
    }
}

void action_reject_message(NotifyNotification *notification, char *action, gpointer data)
{
    Q_UNUSED(notification);
    Q_UNUSED(action);

    EventData* eventData = (EventData*) data;
    Approver* approver = (Approver*) eventData->self;
    if (approver != NULL) {
        approver->onRejectMessage((Tp::ChannelDispatchOperationPtr) eventData->dispatchOp, action);
    }
}

void delete_event_data(gpointer data) {
    if (NULL != data)
    delete (EventData*) data;
}

void Approver::updateNotification(const QContact &contact)
{
    if (!mPendingSnapDecision)
        return;

    QString displayLabel = contact.detail<QContactDisplayLabel>().label();
    QString avatar = contact.detail<QContactAvatar>().imageUrl().toEncoded();

    if (displayLabel.isEmpty()) {
        displayLabel = mDefaultTitle;
    }

    if (avatar.isEmpty()) {
        avatar = mDefaultIcon;
    }

    notify_notification_update(mPendingSnapDecision,
                               displayLabel.toStdString().c_str(),
                               mCachedBody.toStdString().c_str(),
                               avatar.toStdString().c_str());

    GError *error = NULL;
    if (!notify_notification_show(mPendingSnapDecision, &error)) {
        closeSnapDecision();
        qWarning() << "Failed to show snap decision:" << error->message;
        g_error_free (error);
    }
}

void Approver::onChannelReady(Tp::PendingOperation *op)
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

    mChannels.remove(pr);

    // and now set up the contact matching for either greeter mode or regular mode
    if (GreeterContacts::isGreeterMode()) {
        // show the snap decision right away because contact info might never arrive
        showSnapDecision(dispatchOp, channel);
        GreeterContacts::instance()->setContactFilter(QContactPhoneNumber::match(contact->id()));
    } else {
        // try to match the contact info
        QContactFetchRequest *request = new QContactFetchRequest(this);
        request->setFilter(QContactPhoneNumber::match(contact->id()));

        // lambda function to update the notification
        QObject::connect(request, &QContactAbstractRequest::stateChanged, [this, request, dispatchOp, channel]() {
            if (!request || request->state() != QContactAbstractRequest::FinishedState) {
                return;
            }

            QContact contact;

            // create the snap decision only after the contact match finishes
            if (request->contacts().size() > 0) {
                // use the first match
                contact = request->contacts().at(0);

                // Also notify greeter via AccountsService
                GreeterContacts::emitContact(contact);
            }

            showSnapDecision(dispatchOp, channel, contact);
        });

        request->setManager(ContactUtils::sharedManager());
        request->start();
    }
}

void Approver::onApproved(Tp::ChannelDispatchOperationPtr dispatchOp)
{
    closeSnapDecision();

    // forward the channel to the handler
    dispatchOp->handleWith(TELEPHONY_SERVICE_HANDLER);

    // and then launch the dialer-app
    ApplicationUtils::openUrl(QUrl("application:///dialer-app.desktop"));

    mDispatchOps.removeAll(dispatchOp);
}

void Approver::onHangUpAndApproved(Tp::ChannelDispatchOperationPtr dispatchOp)
{
    closeSnapDecision();

    // hangup existing calls
    if (CallManager::instance()->foregroundCall()) {
        CallManager::instance()->foregroundCall()->endCall();
    }

    // forward the channel to the handler
    dispatchOp->handleWith(TELEPHONY_SERVICE_HANDLER);

    // and then launch the dialer-app
    ApplicationUtils::openUrl(QUrl("application:///dialer-app.desktop"));

    mDispatchOps.removeAll(dispatchOp);
}

void Approver::onRejected(Tp::ChannelDispatchOperationPtr dispatchOp)
{
    closeSnapDecision();

    Tp::PendingOperation *claimop = dispatchOp->claim();
    // assume there is just one channel in the dispatchOp for calls
    mChannels[claimop] = dispatchOp->channels().first();
    connect(claimop, SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onClaimFinished(Tp::PendingOperation*)));

    Ringtone::instance()->stopIncomingCallSound();
}

void Approver::onRejectMessage(Tp::ChannelDispatchOperationPtr dispatchOp, const char *action)
{
    if (mRejectActions.contains(action)) {
        QString phoneNumber = dispatchOp->channels().first()->targetContact()->id();
        ChatManager::instance()->sendMessage(QStringList() << phoneNumber, mRejectActions[action],
                                             dispatchOp->account()->uniqueIdentifier());
    }

    onRejected(dispatchOp);
}

bool Approver::showSnapDecision(const Tp::ChannelDispatchOperationPtr dispatchOperation,
                                const Tp::ChannelPtr channel,
                                const QContact &contact)
{
    Tp::ContactPtr telepathyContact = channel->initiatorContact();
    NotifyNotification* notification;
    bool hasCalls = CallManager::instance()->hasCalls();


    /* initial notification */

    EventData* data = new EventData();
    data->self = this;
    data->dispatchOp = dispatchOperation;
    data->channel = channel;
    bool unknownNumber = telepathyContact->id().startsWith("x-ofono-") || telepathyContact->id().isEmpty();

    if (!telepathyContact->id().isEmpty()) {
        if (telepathyContact->id().startsWith("x-ofono-private")) {
            mCachedBody = QString::fromUtf8(C::gettext("Calling from private number"));
        } else if (telepathyContact->id().startsWith("x-ofono-unknown")) {
            mCachedBody = QString::fromUtf8(C::gettext("Calling from unknown number"));
        } else {
            mCachedBody = QString::fromUtf8(C::gettext("Calling from %1")).arg(telepathyContact->id());
        }
    } else {
        mCachedBody = C::gettext("Caller number is not available");
    }

    QString displayLabel;
    QString icon;
    if (!contact.isEmpty()) {
        displayLabel = contact.detail<QContactDisplayLabel>().label();
        icon = contact.detail<QContactAvatar>().imageUrl().toEncoded();
    }

    if (displayLabel.isEmpty()) {
        displayLabel = mDefaultTitle;
    }
    if (icon.isEmpty()) {
        icon = mDefaultIcon;
    }

    notification = notify_notification_new (displayLabel.toStdString().c_str(),
                                            mCachedBody.toStdString().c_str(),
                                            icon.toStdString().c_str());
    notify_notification_set_hint_string(notification,
                                        "x-canonical-snap-decisions",
                                        "true");
    notify_notification_set_hint_string(notification,
                                        "x-canonical-snap-decisions-swipe",
                                        "true");
    notify_notification_set_hint_string(notification,
                                        "x-canonical-private-button-tint",
                                        "true");
    notify_notification_set_hint_string(notification,
                                        "x-canonical-secondary-icon",
                                        "incoming-call");

    QString acceptTitle = hasCalls ? C::gettext("Hold + Answer") :
                                     C::gettext("Accept");
    notify_notification_add_action (notification,
                                    "action_accept",
                                    acceptTitle.toLocal8Bit().data(),
                                    action_accept,
                                    data,
                                    delete_event_data);

#if 0
    // FIXME: re-enable that once we move to fullscreen notifications
    if (hasCalls) {
        notify_notification_add_action (notification,
                                        "action_hangup_and_accept",
                                        C::gettext("End + Answer"),
                                        action_hangup_and_accept,
                                        data,
                                        delete_event_data);
    }
#endif

    notify_notification_add_action (notification,
                                    "action_decline_1",
                                    C::gettext("Decline"),
                                    action_reject,
                                    data,
                                    delete_event_data);

    if (!unknownNumber) {
        notify_notification_add_action(notification,
                                       "action_decline_expansion",
                                       C::gettext("Message & decline"),
                                       action_reject,
                                       data,
                                       delete_event_data);


        Q_FOREACH(const QString &action, mRejectActions.keys()) {
            notify_notification_add_action(notification,
                                           action.toUtf8().data(),
                                           QString("message:%1").arg(mRejectActions[action]).toUtf8().data(),
                                           action_reject_message,
                                           data,
                                           delete_event_data);
        }
    }

    mPendingSnapDecision = notification;

    GError *error = NULL;
    if (!notify_notification_show(notification, &error)) {
        closeSnapDecision();
        qWarning() << "Failed to show snap decision:" << error->message;
        g_error_free (error);
        return false;
    }

    if (hasCalls) {
        ToneGenerator::instance()->playWaitingTone();
    } else {
        // play a ringtone
        Ringtone::instance()->playIncomingCallSound();
    }

    if (!hasCalls && GreeterContacts::instance()->incomingCallVibrate()) {
        mVibrateEffect.setDuration(2000);
        mVibrateEffect.start();
        mVibrateTimer.start();
    }

    return true;
}

Tp::ChannelDispatchOperationPtr Approver::dispatchOperationForIncomingCall()
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

void Approver::processChannels()
{
    Q_FOREACH (Tp::ChannelDispatchOperationPtr dispatchOperation, mDispatchOps) {
        QList<Tp::ChannelPtr> channels = dispatchOperation->channels();
        Q_FOREACH (Tp::ChannelPtr channel, channels) {
            // approve only text channels
            Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(channel);
            if (textChannel.isNull()) {
                continue;
            }

            if (dispatchOperation->possibleHandlers().contains(TELEPHONY_SERVICE_HANDLER)) {
                dispatchOperation->handleWith(TELEPHONY_SERVICE_HANDLER);
                mDispatchOps.removeAll(dispatchOperation);
            }
            // FIXME: this shouldn't happen, but in any case, we need to check what to do when
            // the phone app client is not available
        }
    }
}

void Approver::onClaimFinished(Tp::PendingOperation* op)
{
    if(!op || op->isError()) {
        qDebug() << "onClaimFinished() error";
        // TODO do something
        return;
    }

    Tp::CallChannelPtr callChannel = Tp::CallChannelPtr::dynamicCast(mChannels[op]);
    if (callChannel) {
        Tp::PendingOperation *hangupop = callChannel->hangup(Tp::CallStateChangeReasonUserRequested, TP_QT_ERROR_REJECTED, QString());
        CallNotification::instance()->showNotificationForCall(QStringList() << callChannel->targetContact()->id(), CallNotification::CallRejected);
        mChannels[hangupop] = callChannel;
        connect(hangupop, SIGNAL(finished(Tp::PendingOperation*)),
                this, SLOT(onHangupFinished(Tp::PendingOperation*)));
    }
}

void Approver::onHangupFinished(Tp::PendingOperation* op)
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

void Approver::onCallStateChanged(Tp::CallState state)
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
    } else if (state == Tp::CallStateActive) {
        onApproved(dispatchOperation);
    }
}

void Approver::closeSnapDecision()
{
    if (mPendingSnapDecision != NULL) {
        notify_notification_close(mPendingSnapDecision, NULL);
        mPendingSnapDecision = NULL;
    }

    Ringtone::instance()->stopIncomingCallSound();
    ToneGenerator::instance()->stopWaitingTone();
    mVibrateTimer.stop();
    // WORKAROUND: the ubuntu qt sensors backend does not support setPeriod() and stop(),
    // so we invoke a short vibration to simulate a stop() call
    mVibrateEffect.setDuration(1);
    mVibrateEffect.start();
}

void Approver::onHangupAndAcceptCallRequested()
{
    if (!mPendingSnapDecision) {
        return;
    }

    Tp::ChannelDispatchOperationPtr callDispatchOp = dispatchOperationForIncomingCall();
    if (!callDispatchOp.isNull()) {
        onHangUpAndApproved(callDispatchOp);
    }
}

void Approver::onAcceptCallRequested()
{
    if (!mPendingSnapDecision) {
        return;
    }

    Tp::ChannelDispatchOperationPtr callDispatchOp = dispatchOperationForIncomingCall();
    if (!callDispatchOp.isNull()) {
        onApproved(callDispatchOp);
    }
}

void Approver::onRejectCallRequested()
{
    if (!mPendingSnapDecision) {
        return;
    }

    Tp::ChannelDispatchOperationPtr callDispatchOp = dispatchOperationForIncomingCall();
    if (!callDispatchOp.isNull()) {
        onRejected(callDispatchOp);
    }
}

bool Approver::handleMediaKey(bool doubleClick)
{
    Q_UNUSED(doubleClick)

    // hasCalls gets the value from handler, so even if CallManager isn't ready right now, we know
    // if the event will be handled later
    bool accepted = mPendingSnapDecision || CallManager::instance()->hasCalls();

    // FIXME: Telepathy-qt does not let us know if existing channels are being recovered, 
    // so if this is the first run, call this method again when mSettleTimer is done
    if (mSettleTimer) {
        QObject::connect(mSettleTimer, &QTimer::timeout, [=]() {
            handleMediaKey(doubleClick);
        });
        return accepted;
    }

    // postpone this to avoid blocking dbus method callers
    QMetaObject::invokeMethod(this, "processHandleMediaKey", Qt::QueuedConnection, Q_ARG(bool, doubleClick));
    return accepted;
}

void Approver::processHandleMediaKey(bool doubleClick)
{
    Q_UNUSED(doubleClick)

    if (mPendingSnapDecision) {
        onAcceptCallRequested();
    } else if (CallManager::instance()->hasCalls()) {
        // if there is no incoming call, we have to hangup the current active call
        CallEntry *call =  CallManager::instance()->foregroundCall();
        if (call) {
            call->endCall();
        }
    }
}

