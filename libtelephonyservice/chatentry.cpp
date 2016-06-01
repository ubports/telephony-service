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
    QVariantMap properties = job->property("properties").toMap();
    qDebug() << accountId << messageId << channelObjectPath << properties;
    Tp::TextChannelPtr channel = ChatManager::instance()->channelForObjectPath(channelObjectPath);

    if (channel.isNull()) {
        Q_EMIT messageSendingFailed(accountId, messageId, properties);
        job->deleteLater();
        return;
    }

    // even if sending the message fails, we can use the channel if available
    addChannel(channel);

    if (job->property("status").toInt() == MessageJob::Failed || channel.isNull()) {
        Q_EMIT messageSendingFailed(accountId, messageId, properties);
        job->deleteLater();
        return;
    }

    Q_EMIT messageSent(accountId, messageId, properties);
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

QString ChatEntry::accountId() const
{
    return mAccountId;
}

void ChatEntry::setAccountId(const QString &id)
{
    mAccountId = id;
    Q_EMIT accountIdChanged();
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

void ChatEntry::setChatType(ChatEntry::ChatType type)
{
    mChatType = type;
    Q_EMIT chatTypeChanged();
}

QStringList ChatEntry::participants() const
{
    return mParticipants;
}

void ChatEntry::setParticipants(const QStringList &participants)
{
    mParticipants = participants;
    Q_EMIT participantsChanged();

    // FIXME: we need to invalidate the existing channels & data and start fresh
}

QQmlListProperty<ContactChatState> ChatEntry::chatStates()
{
    return QQmlListProperty<ContactChatState>(this, 0, chatStatesCount, chatStatesAt);
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
    QDBusInterface *sendingJob = new QDBusInterface(TelepathyHelper::instance()->handlerInterface()->service(), objPath,
                                                    "com.canonical.TelephonyServiceHandler.MessageSendingJob");
    qDebug() << sendingJob->isValid();
    sendingJob->dumpObjectInfo();
    connect(sendingJob, SIGNAL(finished()), SLOT(onSendingMessageFinished()));
    QDBusReply<QString> reply = sendingJob->call("Introspect");
    qDebug() << reply.value();
}

void ChatEntry::classBegin()
{
    // nothing to do here
}

void ChatEntry::componentComplete()
{
    QVariantMap properties = generateProperties();
    QList<Tp::TextChannelPtr> channels = ChatManager::instance()->channelForProperties(properties);
    QList<AccountEntry*> accounts;

    if (!channels.isEmpty()) {
        setChannels(channels);
    }

    // now filter out the Phone accounts from the accounts list
    Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->activeAccounts(true)) {
        if (account->type() != AccountEntry::PhoneAccount) {
            accounts << account;
        }
    }

    // now check that we have channels for all !Phone accounts
    // we need channels to be able to show typing notifications
    Q_FOREACH(const Tp::TextChannelPtr &channel, channels) {
        AccountEntry *account = TelepathyHelper::instance()->accountForConnection(channel->connection());
        accounts.removeAll(account);
    }

    // if there is any remaining account, request to start chatting using the account
    Q_FOREACH(AccountEntry *account, accounts) {
        ChatManager::instance()->startChat(account->accountId(), properties);
    }

    connect(ChatManager::instance(), &ChatManager::textChannelAvailable,
            this, &ChatEntry::onTextChannelAvailable);
}

void ChatEntry::setChannels(const QList<Tp::TextChannelPtr> &channels)
{
    Q_FOREACH(const Tp::TextChannelPtr &channel, channels) {
        addChannel(channel);
    }
}

void ChatEntry::addChannel(const Tp::TextChannelPtr &channel)
{
    qDebug() << "adding channel" << channel->objectPath();
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

QVariantMap ChatEntry::generateProperties() const
{
    QVariantMap properties;

    properties["participantIds"] = participants();
    properties["chatType"] = (int)chatType();
    properties["chatId"] = chatId();
    properties["accountId"] = accountId();

    return properties;
}

void ChatEntry::onTextChannelAvailable(const Tp::TextChannelPtr &channel)
{
    if (ChatManager::channelMatchProperties(channel, generateProperties())) {
        addChannel(channel);
    }
}
