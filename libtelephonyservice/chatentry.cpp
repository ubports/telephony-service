/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "telepathyhelper.h"
#include "accountentry.h"
#include "chatentry.h"
#include "chatmanager.h"

// FIXME: move this class to libtelephonyservice
#include "handler/messagejob.h"

#include <TelepathyQt/Contact>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/Connection>

Q_DECLARE_METATYPE(ContactChatStates)

ChatEntry::ChatEntry(QObject *parent) :
    QObject(parent),
    mChatType(ChatTypeNone),
    roomInterface(NULL),
    roomConfigInterface(NULL),
    subjectInterface(NULL)
{
    qRegisterMetaType<ContactChatStates>();
}

void ChatEntry::onRoomPropertiesChanged(const QVariantMap &changed,const QStringList &invalidated)
{
    if (changed.contains("RoomName")) {
        setRoomName(changed["RoomName"].toString());
    }
    if (changed.contains("Title")) {
        mTitle = changed["Title"].toString();
        Q_EMIT titleChanged();
    }
}

void ChatEntry::onSendingMessageFinished()
{
    QDBusInterface *job = qobject_cast<QDBusInterface*>(sender());
    if (!job) {
        return;
    }

    QString accountId = job->property("accountId").toString();
    QString messageId = job->property("messageId").toString();
    QString channelObjectPath = job->property("channelObjectPath").toString();
    Tp::TextChannelPtr channel = ChatManager::instance()->channelForObjectPath(channelObjectPath);

    if (channel.isNull()) {
        Q_EMIT messageSendingFailed(accountId, messageId);
        job->deleteLater();
        return;
    }

    // even if sending the message fails, we can use the channel if available
    addChannel(channel);

    if (job->property("status").toInt() == MessageJob::Failed || channel.isNull()) {
        Q_EMIT messageSendingFailed(accountId, messageId);
        job->deleteLater();
        return;
    }

    Q_EMIT messageSent(accountId, messageId);
    job->deleteLater();
}

QString ChatEntry::roomName() const
{
    return mRoomName;
}

void ChatEntry::setRoomName(const QString &name)
{
    mRoomName = name;
    Q_EMIT roomNameChanged();
    // FIXME: we need to invalidate the existing channels & data and start fresh
}

QString ChatEntry::title() const
{
    return mTitle;
}

ChatEntry::~ChatEntry()
{
    QMap<QString, ContactChatState*> tmp = mChatStates;
    mChatStates.clear();
    Q_EMIT chatStatesChanged();
    QMapIterator<QString, ContactChatState*> it(tmp);
    while (it.hasNext()) {
        it.next();
        delete it.value();
    }

    if (roomInterface) {
        roomInterface->deleteLater();
    }
    if (roomConfigInterface) {
        roomConfigInterface->deleteLater();
    }
    if (subjectInterface) {
        subjectInterface->deleteLater();
    }
}

QString ChatEntry::chatId() const
{
    return mChatId;
}

void ChatEntry::setChatId(const QString &id)
{
    mChatId = id;
    Q_EMIT chatIdChanged();
    // FIXME: we need to invalidate the existing channels & data and start fresh
}

void ChatEntry::onChatStateChanged(const Tp::ContactPtr &contact, Tp::ChannelChatState state)
{
    if (mChatStates.contains(contact->id())) {
        mChatStates[contact->id()]->setState(state);
        return;
    }

    ContactChatState *newState = new ContactChatState(contact->id(), state);
    mChatStates[contact->id()] = newState;
    Q_EMIT chatStatesChanged();
}

ChatEntry::ChatType ChatEntry::chatType() const
{
    return mChatType;
}

QStringList ChatEntry::participants() const
{
    return mParticipants;
}

QQmlListProperty<ContactChatState> ChatEntry::chatStates()
{
    return QQmlListProperty<ContactChatState>(this, 0, chatStatesCount, chatStatesAt);
}

void ChatEntry::setParticipants(const QStringList &participants)
{
    mParticipants = participants;
    Q_EMIT participantsChanged();

    // FIXME: we need to invalidate the existing channels & data and start fresh
}

int ChatEntry::chatStatesCount(QQmlListProperty<ContactChatState> *p)
{
    ChatEntry *entry = qobject_cast<ChatEntry*>(p->object);
    if (!entry) {
        return 0;
    }
    return entry->mChatStates.count();
}

ContactChatState *ChatEntry::chatStatesAt(QQmlListProperty<ContactChatState> *p, int index)
{
    ChatEntry *entry = qobject_cast<ChatEntry*>(p->object);
    if (!entry) {
        return 0;
    }
    return entry->mChatStates.values()[index];
}

void ChatEntry::sendMessage(const QString &accountId, const QString &message, const QVariant &attachments, const QVariantMap &properties)
{
    QString objPath = ChatManager::instance()->sendMessage(accountId, message, attachments, properties);
    QDBusInterface *sendingJob = new QDBusInterface(TelepathyHelper::instance()->handlerInterface()->service(),
                                                    objPath,
                                                    TelepathyHelper::instance()->handlerInterface()->interface(),
                                                    QDBusConnection::sessionBus(),
                                                    this);
    connect(sendingJob, SIGNAL(finished()), SLOT(onSendingMessageFinished()));
}

void ChatEntry::classBegin()
{
    // nothing to do here
}

void ChatEntry::componentComplete()
{
    // FIXME: implement
    QVariantMap properties;

    properties["participantIds"] = participants();

    QList<Tp::TextChannelPtr> channels = ChatManager::instance()->channelForProperties(properties);
    if (!channels.isEmpty()) {
        setChannels(channels);
    } else {
        // FIXME: start chatting with all accounts that support typing notifications
    }
}

void ChatEntry::setChannels(const QList<Tp::TextChannelPtr> &channels)
{
    Q_FOREACH(const Tp::TextChannelPtr &channel, channels) {
        addChannel(channel);
    }
}

void ChatEntry::addChannel(const Tp::TextChannelPtr &channel)
{
    if (mChannels.contains(channel)) {
        return;
    }

    roomInterface = channel->optionalInterface<Tp::Client::ChannelInterfaceRoomInterface>();
    roomConfigInterface = channel->optionalInterface<Tp::Client::ChannelInterfaceRoomConfigInterface>();
    subjectInterface = channel->optionalInterface<Tp::Client::ChannelInterfaceSubjectInterface>();

    if (roomInterface) {
        roomInterface->setMonitorProperties(true);
        connect(roomInterface, SIGNAL(propertiesChanged(const QVariantMap &,const QStringList &)),
                               SLOT(onRoomPropertiesChanged(const QVariantMap &,const QStringList &)));
    }
    if (roomConfigInterface) {
        roomConfigInterface->setMonitorProperties(true);
        connect(roomConfigInterface, SIGNAL(propertiesChanged(const QVariantMap &,const QStringList &)),
                                     SLOT(onRoomPropertiesChanged(const QVariantMap &,const QStringList &)));
    }
    if (subjectInterface) {
        subjectInterface->setMonitorProperties(true);
        connect(subjectInterface, SIGNAL(propertiesChanged(const QVariantMap &,const QStringList &)),
                                  SLOT(onRoomPropertiesChanged(const QVariantMap &,const QStringList &)));
    }

    connect(channel.data(), SIGNAL(chatStateChanged(const Tp::ContactPtr &, Tp::ChannelChatState)),
                            this, SLOT(onChatStateChanged(const Tp::ContactPtr &,Tp::ChannelChatState)));
    connect(channel.data(), SIGNAL(groupMembersChanged(const Tp::Contacts &, const Tp::Contacts &, const Tp::Contacts &,
            const Tp::Contacts &, const Tp::Channel::GroupMemberChangeDetails &)), this, SIGNAL(participantsChanged()));

    Q_FOREACH (Tp::ContactPtr contact, channel->groupContacts(false)) {
        // FIXME: we should not create new chat states for contacts already found in previous channels
        ContactChatState *state = new ContactChatState(contact->id(), channel->chatState(contact));
        mChatStates[contact->id()] = state;
    }

    // now fill the properties with the data from the channel
    if (chatType() != (ChatType)channel->targetHandleType()) {
        setChatType((ChatType)channel->targetHandleType());
    }
    if (chatType() == ChatTypeRoom && mChatId != channel->targetId()) {
        setChatId(channel->targetId());
    }

    mChannels << channel;
}

void ChatEntry::setChatType(ChatEntry::ChatType type)
{
    mChatType = type;
    Q_EMIT chatTypeChanged();
}
