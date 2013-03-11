/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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

#include "chatmanager.h"
#include "contactmodel.h"
#include "telepathyhelper.h"
#include "config.h"

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>

ChatManager::ChatManager(QObject *parent)
: QObject(parent)
{
    // track when the account becomes available
    connect(TelepathyHelper::instance(), SIGNAL(accountReady()), SLOT(onAccountReady()));
    // track when the connection becomes available
    connect(TelepathyHelper::instance(), SIGNAL(connectionChanged()), SLOT(onAccountReady()));
}

void ChatManager::onAccountReady()
{
    if (!TelepathyHelper::instance()->account() || !TelepathyHelper::instance()->account()->connection()) {
        return;
    }
    Q_FOREACH(const QString &number, mChatPending) {
        startChat(number);
    }
    mChatPending.clear();
}

ChatManager *ChatManager::instance()
{
    static ChatManager *manager = new ChatManager();
    return manager;
}

bool ChatManager::isChattingToContact(const QString &phoneNumber)
{
    // if this is not running in the telephony application instance,
    // we only send messages directly if the app is not running

    // if it is running, we assume we can chat to a contact directly
    if (!isTelephonyApplicationInstance() &&
        isTelephonyApplicationRunning()) {
        return true;
    }

    return !existingChat(phoneNumber).isNull();
}

void ChatManager::startChat(const QString &phoneNumber)
{
    if (!TelepathyHelper::instance()->account() || !TelepathyHelper::instance()->account()->connection()) {
        mChatPending << phoneNumber;
        return;
    }

    if (!isChattingToContact(phoneNumber)) {
        // Request the contact to start chatting to
        Tp::AccountPtr account = TelepathyHelper::instance()->account();
        connect(account->connection()->contactManager()->contactsForIdentifiers(QStringList() << phoneNumber),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onContactsAvailable(Tp::PendingOperation*)));
    }
}

void ChatManager::endChat(const QString &phoneNumber)
{
    Tp::TextChannelPtr channel = existingChat(phoneNumber);
    if (channel.isNull()) {
        return;
    }

    // the phoneNumber might be formatted differently from the phone number used as the key
    // so use the one from the channel to remove the entries.
    QString id = channel->targetContact()->id();
    channel->requestClose();
    mChannels.remove(id);
    mContacts.remove(id);

    Q_EMIT unreadMessagesChanged(id);
}

void ChatManager::sendMessage(const QString &phoneNumber, const QString &message)
{
    // if the telephony-app is running, just send a message using it
    if (!isTelephonyApplicationInstance() &&
        isTelephonyApplicationRunning()) {
        QDBusInterface telephonyApp("com.canonical.TelephonyApp",
                                    "/com/canonical/TelephonyApp",
                                    "com.canonical.TelephonyApp");

        telephonyApp.call("SendMessage", phoneNumber, message);
        Q_EMIT messageSent(phoneNumber, message);
        return;
    }

    Tp::TextChannelPtr channel = existingChat(phoneNumber);
    if (channel.isNull()) {
        mPendingMessages[phoneNumber] = message;
        startChat(phoneNumber);
        return;
    }

    connect(channel->send(message),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onMessageSent(Tp::PendingOperation*)));
}

void ChatManager::acknowledgeMessages(const QString &phoneNumber)
{
    Tp::TextChannelPtr channel = existingChat(phoneNumber);
    if (channel.isNull()) {
        return;
    }

    channel->acknowledge(channel->messageQueue());
}

int ChatManager::unreadMessagesCount() const
{
    int count = 0;
    Q_FOREACH(const Tp::TextChannelPtr &channel, mChannels.values()) {
        count += channel->messageQueue().count();
    }

    return count;
}

int ChatManager::unreadMessages(const QString &phoneNumber)
{
    Tp::TextChannelPtr channel = existingChat(phoneNumber);
    if (channel.isNull()) {
        return 0;
    }

    return channel->messageQueue().count();
}

void ChatManager::onTextChannelAvailable(Tp::TextChannelPtr channel)
{
    QString id = channel->targetContact()->id();
    mChannels[id] = channel;
    connect(channel.data(),
            SIGNAL(pendingMessageRemoved(const Tp::ReceivedMessage&)),
            SLOT(onPendingMessageRemoved(const Tp::ReceivedMessage&)));

    Q_EMIT chatReady(id);
    Q_EMIT unreadMessagesChanged(id);

    // if there is a pending message for this number, send it
    if (mPendingMessages.contains(id)) {
        sendMessage(id, mPendingMessages[id]);
        mPendingMessages.remove(id);
    }
}

void ChatManager::onMessageReceived(const Tp::ReceivedMessage &message)
{
    // ignore delivery reports for now
    if (message.isDeliveryReport()) {
        return;
    }

    Q_EMIT messageReceived(message.sender()->id(), message.text(), message.received(), message.messageToken(), true);

    Q_EMIT unreadMessagesChanged(message.sender()->id());;
}

void ChatManager::onPendingMessageRemoved(const Tp::ReceivedMessage &message)
{
    // emit the signal saying the unread messages for a specific number has changed
    Q_EMIT unreadMessagesChanged(message.sender()->id());
}

void ChatManager::onMessageSent(Tp::PendingOperation *op)
{
    Tp::PendingSendMessage *psm = qobject_cast<Tp::PendingSendMessage*>(op);
    if(!psm) {
        qWarning() << "The pending object was not a pending operation:" << op;
        return;
    }

    if (psm->isError()) {
        qWarning() << "Error sending message:" << psm->errorName() << psm->errorMessage();
        return;
    }

    Q_EMIT messageSent(psm->channel()->targetContact()->id(), psm->message().text());
}

void ChatManager::acknowledgeMessage(const QString &phoneNumber, const QString &messageId)
{
    Tp::TextChannelPtr channel = existingChat(phoneNumber);
    if(channel.isNull() || messageId.isNull()) {
        return;
    }

    Q_FOREACH(const Tp::ReceivedMessage &message, channel->messageQueue()) {
        if (message.messageToken() == messageId) {
            channel->acknowledge(QList<Tp::ReceivedMessage>() << message);
            break;
        }
    }
}

Tp::TextChannelPtr ChatManager::existingChat(const QString &phoneNumber)
{
    Tp::TextChannelPtr channel;
    Q_FOREACH(const QString &key, mChannels.keys()) {
        if (ContactModel::comparePhoneNumbers(key, phoneNumber)) {
            channel = mChannels[key];
            break;
        }
    }

    return channel;
}

void ChatManager::onContactsAvailable(Tp::PendingOperation *op)
{
    Tp::PendingContacts *pc = qobject_cast<Tp::PendingContacts*>(op);

    if (!pc) {
        qCritical() << "The pending object is not a Tp::PendingContacts";
        return;
    }

    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    // start chatting to the contacts
    Q_FOREACH(Tp::ContactPtr contact, pc->contacts()) {
        QString handler = TelepathyHelper::instance()->channelHandler()->property("clientName").toString();
        account->ensureTextChat(contact, QDateTime::currentDateTime(), handler);

        // hold the ContactPtr to make sure its refcounting stays bigger than 0
        mContacts[contact->id()] = contact;
    }
}
