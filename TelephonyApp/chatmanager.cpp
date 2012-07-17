/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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

#include "chatmanager.h"
#include "contactmodel.h"
#include "telepathyhelper.h"

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>

ChatManager::ChatManager(QObject *parent)
: QObject(parent)
{
}

ChatManager *ChatManager::instance()
{
    static ChatManager *manager = new ChatManager();
    return manager;
}

bool ChatManager::isChattingToContact(const QString &contactId)
{
    return mChannels.contains(contactId);
}

void ChatManager::startChat(const QString &contactId)
{
    if (!mChannels.contains(contactId)) {
        // Request the contact to start chatting to
        Tp::AccountPtr account = TelepathyHelper::instance()->account();
        connect(account->connection()->contactManager()->contactsForIdentifiers(QStringList() << contactId),
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onContactsAvailable(Tp::PendingOperation*)));
    }
}

void ChatManager::endChat(const QString &contactId)
{
    if (!mChannels.contains(contactId))
        return;

    mChannels[contactId]->requestClose();
    mChannels.remove(contactId);
    mContacts.remove(contactId);

    emit unreadMessagesChanged(contactId);
}

void ChatManager::sendMessage(const QString &contactId, const QString &message)
{
    if (!mChannels.contains(contactId))
        return;

    mChannels[contactId]->send(message);
    emit messageSent(contactId, message);
}

void ChatManager::acknowledgeMessages(const QString &contactId)
{
    if (!mChannels.contains(contactId))
        return;

    Tp::TextChannelPtr channel = mChannels[contactId];
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

int ChatManager::unreadMessages(const QString &contactId)
{
    int count = 0;
    Q_FOREACH(const Tp::TextChannelPtr &channel, mChannels.values()) {
        if (ContactModel::instance()->comparePhoneNumbers(contactId, channel->targetContact()->id())) {
            count += channel->messageQueue().count();
        }
    }

    return count;
}

void ChatManager::onTextChannelAvailable(Tp::TextChannelPtr channel)
{
    mChannels[channel->targetContact()->id()] = channel;

    connect(channel.data(),
            SIGNAL(pendingMessageRemoved(const Tp::ReceivedMessage&)),
            SLOT(onPendingMessageRemoved(const Tp::ReceivedMessage&)));

    emit chatReady(channel->targetContact()->id());
    emit unreadMessagesChanged(channel->targetContact()->id());
}

void ChatManager::onMessageReceived(const Tp::ReceivedMessage &message)
{
    emit messageReceived(message.sender()->id(), message.text());
    emit unreadMessagesChanged(message.sender()->id());;
}

void ChatManager::onPendingMessageRemoved(const Tp::ReceivedMessage &message)
{
    // emit the signal saying the unread messages for a specific number has changed
    emit unreadMessagesChanged(message.sender()->id());
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
        account->ensureTextChat(contact, QDateTime::currentDateTime(), "org.freedesktop.Telepathy.Client.TelephonyApp");

        // hold the ContactPtr to make sure its refcounting stays bigger than 0
        mContacts[contact->id()] = contact;
    }
}
