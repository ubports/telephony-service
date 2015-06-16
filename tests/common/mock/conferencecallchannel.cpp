/**
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *     Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 */

#include "conferencecallchannel.h"
#include "callchannel.h"


MockConferenceCallChannel::MockConferenceCallChannel(MockConnection *conn, QList<QDBusObjectPath> callChannels, QObject *parent):
    mRequestedHangup(false),
    mConnection(conn),
    mDtmfLock(false),
    mCallChannels(callChannels)
{

    Q_FOREACH(MockCallChannel *channel, mConnection->callChannels().values()) {
        if (channel->callState() == Tp::CallStateActive) {
            QDBusObjectPath path(channel->baseChannel()->objectPath());
            mCallChannels << path;
        }
    }

    Tp::BaseChannelPtr baseChannel = Tp::BaseChannel::create(mConnection, TP_QT_IFACE_CHANNEL_TYPE_CALL, Tp::HandleTypeNone, 0);
    Tp::BaseChannelCallTypePtr callType = Tp::BaseChannelCallType::create(baseChannel.data(),
                                                                          true,
                                                                          Tp::StreamTransportTypeUnknown,
                                                                          true,
                                                                          false, "","");
    baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(callType));

    mHoldIface = Tp::BaseChannelHoldInterface::create();
    mHoldIface->setSetHoldStateCallback(Tp::memFun(this,&MockConferenceCallChannel::onHoldStateChanged));

    mMuteIface = Tp::BaseCallMuteInterface::create();
    mMuteIface->setSetMuteStateCallback(Tp::memFun(this,&MockConferenceCallChannel::onMuteStateChanged));

    mSpeakerIface = BaseChannelSpeakerInterface::create();
    mSpeakerIface->setTurnOnSpeakerCallback(Tp::memFun(this,&MockConferenceCallChannel::onTurnOnSpeaker));

    mConferenceIface = Tp::BaseChannelConferenceInterface::create(mCallChannels);

    mMergeableIface = Tp::BaseChannelMergeableConferenceInterface::create();
    mMergeableIface->setMergeCallback(Tp::memFun(this,&MockConferenceCallChannel::onMerge));

    baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(mHoldIface));
    baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(mMuteIface));
    baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(mSpeakerIface));
    baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(mConferenceIface));
    baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(mMergeableIface));

    mBaseChannel = baseChannel;
    mCallChannel = Tp::BaseChannelCallTypePtr::dynamicCast(mBaseChannel->interface(TP_QT_IFACE_CHANNEL_TYPE_CALL));

    mCallChannel->setHangupCallback(Tp::memFun(this,&MockConferenceCallChannel::onHangup));

    Tp::CallStateReason reason;
    QVariantMap stateDetails;
    reason.actor =  0;
    reason.reason = Tp::CallStateChangeReasonUserRequested;
    reason.message = "";
    reason.DBusReason = "";

    mCallChannel->setCallState(Tp::CallStateActive, 0, reason, stateDetails);

    QObject::connect(mConnection, SIGNAL(channelSplitted(QDBusObjectPath)), SLOT(onChannelSplitted(QDBusObjectPath)));

    // init must be called after initialization, otherwise we will have no object path registered.
    QTimer::singleShot(0, this, SLOT(init()));
}

Tp::BaseChannelPtr MockConferenceCallChannel::baseChannel()
{
    return mBaseChannel;
}

void MockConferenceCallChannel::onMerge(const QDBusObjectPath &channel, Tp::DBusError *error)
{
    if (!mCallChannels.contains(channel)) {
        mCallChannels << channel;
        mConferenceIface->mergeChannel(channel, 0, QVariantMap());
        Q_EMIT channelMerged(channel.path());
    }
}

void MockConferenceCallChannel::onChannelSplitted(const QDBusObjectPath &path)
{
    if (mCallChannels.contains(path)) {
        mCallChannels.removeAll(path);
        mConferenceIface->removeChannel(path, QVariantMap());
    }
    if (mCallChannels.size() == 1) {
        // remove the call channel from the conference before closing it.
        mConferenceIface->removeChannel(mCallChannels.takeFirst(), QVariantMap());

        Tp::CallStateReason reason;
        QVariantMap stateDetails;
        reason.actor =  0;
        reason.reason = Tp::CallStateChangeReasonUserRequested;
        reason.message = "";
        reason.DBusReason = "";

        mCallChannel->setCallState(Tp::CallStateEnded, 0, reason, stateDetails);
        mBaseChannel->close();
    }
}

void MockConferenceCallChannel::onTurnOnSpeaker(bool active, Tp::DBusError *error)
{
    //mConnection->setSpeakerMode(active);
    // FIXME: reimplement
}

void MockConferenceCallChannel::onHangup(uint reason, const QString &detailedReason, const QString &message, Tp::DBusError *error)
{
    //FIXME: reimplement
    Tp::CallStateReason theReason;
    QVariantMap stateDetails;
    theReason.actor =  0;
    theReason.reason = reason;
    theReason.message = message;
    theReason.DBusReason = "";

    mCallChannel->setCallState(Tp::CallStateEnded, 0, theReason, stateDetails);
    mBaseChannel->close();
}

void MockConferenceCallChannel::init()
{
    QVariantMap stateDetails;
    Tp::CallStateReason reason;

    mObjPath = mBaseChannel->objectPath();

    reason.actor =  0;
    reason.reason = Tp::CallStateChangeReasonProgressMade;
    reason.message = "";
    reason.DBusReason = "";

    mCallChannel->setCallState(Tp::CallStateActive, 0, reason, stateDetails);

    mDTMFIface = Tp::BaseCallContentDTMFInterface::create();

    mDTMFIface->setStartToneCallback(Tp::memFun(this,&MockConferenceCallChannel::onDTMFStartTone));
    mDTMFIface->setStopToneCallback(Tp::memFun(this,&MockConferenceCallChannel::onDTMFStopTone));

    QObject::connect(mBaseChannel.data(), SIGNAL(closed()), this, SLOT(deleteLater()));
    //QObject::connect(mConnection->callVolume(), SIGNAL(mutedChanged(bool)), SLOT(onOfonoMuteChanged(bool)));
    QObject::connect(mConnection, SIGNAL(speakerModeChanged(bool)), mSpeakerIface.data(), SLOT(setSpeakerMode(bool)));
    //QObject::connect(mConnection->voiceCallManager(), SIGNAL(sendTonesComplete(bool)), SLOT(onDtmfComplete(bool)));

    //mSpeakerIface->setSpeakerMode(mConnection->speakerMode());
    QObject::connect(mConnection, SIGNAL(channelSplitted(const QDBusObjectPath&)), this, SLOT(onChannelSplitted(const QDBusObjectPath&)));
    QObject::connect(mConnection, SIGNAL(channelHangup(const QDBusObjectPath&)), this, SLOT(onChannelSplitted(const QDBusObjectPath&)));

    Q_EMIT initialized();
}

void MockConferenceCallChannel::onOfonoMuteChanged(bool mute)
{
    Tp::LocalMuteState state = mute ? Tp::LocalMuteStateMuted : Tp::LocalMuteStateUnmuted;
    mMuteIface->setMuteState(state);
}

void MockConferenceCallChannel::setConferenceActive(bool active)
{
    if (active) {
        mHoldIface->setHoldState(Tp::LocalHoldStateUnheld, Tp::LocalHoldStateReasonNone);
    } else {
        mHoldIface->setHoldState(Tp::LocalHoldStateHeld, Tp::LocalHoldStateReasonNone);
    }
}

void MockConferenceCallChannel::onHoldStateChanged(const Tp::LocalHoldState &state, const Tp::LocalHoldStateReason &reason, Tp::DBusError *error)
{
    /*if (state == Tp::LocalHoldStateHeld && mHoldIface->getHoldState() == Tp::LocalHoldStateUnheld) {
        mConnection->voiceCallManager()->swapCalls();
    } else if (state == Tp::LocalHoldStateUnheld && mHoldIface->getHoldState() == Tp::LocalHoldStateHeld) {
        mConnection->voiceCallManager()->swapCalls();
    }*/

    // FIXME: reimplement
}

void MockConferenceCallChannel::onMuteStateChanged(const Tp::LocalMuteState &state, Tp::DBusError *error)
{
    /*if (state == Tp::LocalMuteStateMuted) {
        mConnection->callVolume()->setMuted(true);
    } else if (state == Tp::LocalMuteStateUnmuted) {
        mConnection->callVolume()->setMuted(false);
    }*/

    // FIXME: reimplement
}

void MockConferenceCallChannel::sendNextDtmf()
{
    /*if (mDtmfLock) {
        return;
    }
    if (!mDtmfPendingStrings.isEmpty()) {
        mDtmfLock = true;
        mConnection->voiceCallManager()->sendTones(mDtmfPendingStrings.front());
    }*/
    // FIXME: reimplement
}

void MockConferenceCallChannel::onDtmfComplete(bool success)
{
    mDtmfLock = false;
    if (success) {
        mDtmfPendingStrings.removeFirst();
       if (mDtmfPendingStrings.isEmpty()) {
           return;
       }
       sendNextDtmf();
    } else {
        QTimer::singleShot(1000, this, SLOT(sendNextDtmf()));
    }
}

void MockConferenceCallChannel::onDTMFStartTone(uchar event, Tp::DBusError *error)
{
    QString finalString;
    if (event == 10) {
        finalString = "*";
    } else if (event == 11) {
        finalString = "#";
    } else {
        finalString = QString::number(event);
    }

    qDebug() << "start tone" << finalString;
    // we can't append to the first item in the queue as it is being sent and
    // we dont know yet if it will succeed or not.
    if (mDtmfPendingStrings.count() > 1) {
        mDtmfPendingStrings[1] += finalString;
    } else {
        mDtmfPendingStrings << finalString;
    }
    sendNextDtmf();
}

void MockConferenceCallChannel::onDTMFStopTone(Tp::DBusError *error)
{
}

MockConferenceCallChannel::~MockConferenceCallChannel()
{
    qDebug() << "conference call channel closed";
    // TODO - for some reason the object is not being removed
    mConnection->dbusConnection().unregisterObject(mObjPath, QDBusConnection::UnregisterTree);
}
