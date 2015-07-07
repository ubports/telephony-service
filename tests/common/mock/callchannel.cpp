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
 *          Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 */

#include "callchannel.h"

MockCallChannel::MockCallChannel(MockConnection *conn, QString phoneNumber, QString state, uint targetHandle, QObject *parent):
    QObject(parent),
    mIncoming(false),
    mRequestedHangup(false),
    mConnection(conn),
    mPhoneNumber(phoneNumber),
    mTargetHandle(targetHandle),
    mState(state)
{
    Tp::BaseChannelPtr baseChannel = Tp::BaseChannel::create(mConnection, TP_QT_IFACE_CHANNEL_TYPE_CALL, Tp::HandleTypeContact, targetHandle);
    Tp::BaseChannelCallTypePtr callType = Tp::BaseChannelCallType::create(baseChannel.data(),
                                                                          true,
                                                                          Tp::StreamTransportTypeUnknown,
                                                                          true,
                                                                          false, "","");
    baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(callType));

    mHoldIface = Tp::BaseChannelHoldInterface::create();
    mHoldIface->setSetHoldStateCallback(Tp::memFun(this,&MockCallChannel::onHoldStateChanged));

    mMuteIface = Tp::BaseCallMuteInterface::create();
    mMuteIface->setSetMuteStateCallback(Tp::memFun(this,&MockCallChannel::onMuteStateChanged));

    mSplittableIface = Tp::BaseChannelSplittableInterface::create();
    mSplittableIface->setSplitCallback(Tp::memFun(this,&MockCallChannel::onSplit));

    baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(mHoldIface));
    baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(mMuteIface));
    baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(mSplittableIface));

    mBaseChannel = baseChannel;
    mCallChannel = Tp::BaseChannelCallTypePtr::dynamicCast(mBaseChannel->interface(TP_QT_IFACE_CHANNEL_TYPE_CALL));

    mCallChannel->setHangupCallback(Tp::memFun(this,&MockCallChannel::onHangup));
    mCallChannel->setAcceptCallback(Tp::memFun(this,&MockCallChannel::onAccept));

    // init must be called after initialization, otherwise we will have no object path registered.
    QTimer::singleShot(0, this, SLOT(init()));
}

void MockCallChannel::onHangup(uint reason, const QString &detailedReason, const QString &message, Tp::DBusError *error)
{
    // TODO: use the parameters sent by telepathy
    mRequestedHangup = true;
    setCallState("disconnected");
}

void MockCallChannel::onAccept(Tp::DBusError*)
{
    setCallState("active");
}

void MockCallChannel::init()
{
    Tp::CallMemberMap memberFlags;
    Tp::HandleIdentifierMap identifiers;
    QVariantMap stateDetails;
    Tp::CallStateReason reason;

    mIncoming = mState == "incoming" || mState == "waiting";

    identifiers[mTargetHandle] = mPhoneNumber;
    reason.actor =  0;
    reason.reason = Tp::CallStateChangeReasonProgressMade;
    reason.message = "";
    reason.DBusReason = "";
    if (mIncoming) {
        memberFlags[mTargetHandle] = 0;
    } else {
        memberFlags[mTargetHandle] = Tp::CallMemberFlagRinging;
    }

    mCallChannel->setCallState(Tp::CallStateInitialising, 0, reason, stateDetails);

    mCallContent = Tp::BaseCallContent::create(baseChannel()->dbusConnection(), baseChannel().data(), "audio", Tp::MediaStreamTypeAudio, Tp::MediaStreamDirectionNone);

    mDTMFIface = Tp::BaseCallContentDTMFInterface::create();
    mCallContent->plugInterface(Tp::AbstractCallContentInterfacePtr::dynamicCast(mDTMFIface));
    mCallChannel->addContent(mCallContent);

    mDTMFIface->setStartToneCallback(Tp::memFun(this,&MockCallChannel::onDTMFStartTone));
    mDTMFIface->setStopToneCallback(Tp::memFun(this,&MockCallChannel::onDTMFStopTone));

    mCallChannel->setMembersFlags(memberFlags, identifiers, Tp::UIntList(), reason);

    mCallChannel->setCallState(Tp::CallStateInitialised, 0, reason, stateDetails);
    QObject::connect(mBaseChannel.data(), SIGNAL(closed()), this, SLOT(deleteLater()));
}

void MockCallChannel::onOfonoMuteChanged(bool mute)
{
    Tp::LocalMuteState state = mute ? Tp::LocalMuteStateMuted : Tp::LocalMuteStateUnmuted;
    mMuteIface->setMuteState(state);
}

void MockCallChannel::onHoldStateChanged(const Tp::LocalHoldState &state, const Tp::LocalHoldStateReason &reason, Tp::DBusError *error)
{
    if (state == Tp::LocalHoldStateHeld && mState == "active") {
        setCallState("held");
    } else if (state == Tp::LocalHoldStateUnheld && mState == "held") {
        setCallState("active");
    }
}

void MockCallChannel::onMuteStateChanged(const Tp::LocalMuteState &state, Tp::DBusError *error)
{
#if 0
    FIXME: reimplement
    if (state == Tp::LocalMuteStateMuted) {
        mConnection->callVolume()->setMuted(true);
    } else if (state == Tp::LocalMuteStateUnmuted) {
        mConnection->callVolume()->setMuted(false);
    }
#endif
}

void MockCallChannel::onDTMFStartTone(uchar event, Tp::DBusError *error)
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
}

void MockCallChannel::onDTMFStopTone(Tp::DBusError *error)
{
}

void MockCallChannel::onSplit(Tp::DBusError *error)
{
    Q_EMIT splitted();
}

QString MockCallChannel::objectPath() const
{
    return mBaseChannel->objectPath();
}

Tp::CallState MockCallChannel::callState() const
{
    return (Tp::CallState) mCallChannel->callState();
}

MockCallChannel::~MockCallChannel()
{
    qDebug() << "call channel closed";
    // TODO - for some reason the object is not being removed
    mConnection->dbusConnection().unregisterObject(mBaseChannel->objectPath(), QDBusConnection::UnregisterTree);
}

Tp::BaseChannelPtr MockCallChannel::baseChannel()
{
    return mBaseChannel;
}

void MockCallChannel::setCallState(const QString &state)
{
    Tp::CallStateReason reason;
    QVariantMap stateDetails;
    reason.actor =  0;
    reason.reason = Tp::CallStateChangeReasonUserRequested;
    reason.message = "";
    reason.DBusReason = "";
    if (state == "disconnected") {
        qDebug() << "disconnected";
        if (mIncoming && mState == "incoming" && !mRequestedHangup) {
            reason.reason = Tp::CallStateChangeReasonNoAnswer;
        }
        mCallChannel->setCallState(Tp::CallStateEnded, 0, reason, stateDetails);
        mBaseChannel->close();
    } else if (state == "active") {
        qDebug() << "active";
        mHoldIface->setHoldState(Tp::LocalHoldStateUnheld, Tp::LocalHoldStateReasonNone);
        if (mState == "dialing" || mState == "alerting" ||
                mState == "incoming") {
            mCallChannel->setCallState(Tp::CallStateAccepted, 0, reason, stateDetails);
        }
        mCallChannel->setCallState(Tp::CallStateActive, 0, reason, stateDetails);
    } else if (state == "held") {
        mHoldIface->setHoldState(Tp::LocalHoldStateHeld, Tp::LocalHoldStateReasonNone);
        qDebug() << "held";
    } else if (state == "dialing") {
        qDebug() << "dialing";
    } else if (state == "alerting") {
        qDebug() << "alerting";
    } else if (state == "incoming") {
        mIncoming = true;
        qDebug() << "incoming";
    } else if (state == "waiting") {
        mIncoming = true;
        qDebug() << "waiting";
    }

    mState = state;

    Q_EMIT callStateChanged(this, state);
}
