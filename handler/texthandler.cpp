/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
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

#include "texthandler.h"
#include "phoneutils.h"
#include "telepathyhelper.h"
#include "config.h"

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>

template<> bool qMapLessThanKey<QStringList>(const QStringList &key1, const QStringList &key2) 
{ 
    return key1.size() > key2.size();  // sort by operator> !
}

TextHandler::TextHandler(QObject *parent)
: QObject(parent)
{
    // track when the account becomes available
    connect(TelepathyHelper::instance(),
            SIGNAL(connectedChanged()),
            SLOT(onConnectedChanged()));
}

void TextHandler::onConnectedChanged()
{
    if (!TelepathyHelper::instance()->connected()) {
        return;
    }

    // now check which accounts are connected
    Q_FOREACH(const Tp::AccountPtr &account, TelepathyHelper::instance()->accounts()) {
        QString accountId = account->uniqueIdentifier();
        if (!TelepathyHelper::instance()->isAccountConnected(account) || !mPendingMessages.contains(accountId)) {
            continue;
        }

        // create text channels to send the pending messages
        Q_FOREACH(const QStringList& phoneNumbers, mPendingMessages[accountId].keys()) {
            startChat(phoneNumbers, accountId);
        }
    }

}

TextHandler *TextHandler::instance()
{
    static TextHandler *handler = new TextHandler();
    return handler;
}

void TextHandler::startChat(const QStringList &phoneNumbers, const QString &accountId)
{
    // Request the contact to start chatting to
    // FIXME: make it possible to select which account to use, for now, pick the first one
    Tp::AccountPtr account = TelepathyHelper::instance()->accountForId(accountId);
    if (!TelepathyHelper::instance()->isAccountConnected(account)) {
        qCritical() << "The selected account does not have a connection. AccountId:" << accountId;
        return;
    }

    connect(account->connection()->contactManager()->contactsForIdentifiers(phoneNumbers),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onContactsAvailable(Tp::PendingOperation*)));
}

void TextHandler::startChat(const Tp::AccountPtr &account, const Tp::Contacts &contacts)
{
    if (contacts.size() == 1) {
        account->ensureTextChat(contacts.values()[0], QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler");
    } else {
        account->createConferenceTextChat(QList<Tp::ChannelPtr>(), contacts.toList(), QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler");
    }
    // start chatting to the contacts
    Q_FOREACH(Tp::ContactPtr contact, contacts) {
        // hold the ContactPtr to make sure its refcounting stays bigger than 0
        mContacts[contact->id()] = contact;
    }
}

void TextHandler::sendMessage(const QStringList &phoneNumbers, const QString &message, const QString &accountId)
{
    Tp::AccountPtr account = TelepathyHelper::instance()->accountForId(accountId);
    if (!TelepathyHelper::instance()->isAccountConnected(account)) {
        mPendingMessages[accountId][phoneNumbers].append(message);
        return;
    }

    Tp::TextChannelPtr channel = existingChat(phoneNumbers, accountId);
    if (channel.isNull()) {
        mPendingMessages[accountId][phoneNumbers].append(message);
        startChat(phoneNumbers, accountId);
        return;
    }

    connect(channel->send(message),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onMessageSent(Tp::PendingOperation*)));
}

void TextHandler::acknowledgeMessages(const QStringList &phoneNumbers, const QStringList &messageIds, const QString &accountId)
{
    Tp::TextChannelPtr channel = existingChat(phoneNumbers, accountId);
    if (channel.isNull()) {
        return;
    }

    QList<Tp::ReceivedMessage> messagesToAck;
    Q_FOREACH(const Tp::ReceivedMessage &message, channel->messageQueue()) {
        if (messageIds.contains(message.messageToken())) {
            messagesToAck.append(message);
        }
    }

    channel->acknowledge(messagesToAck);
}

void TextHandler::onTextChannelAvailable(Tp::TextChannelPtr channel)
{
    Tp::AccountPtr account = TelepathyHelper::instance()->accountForConnection(channel->connection());
    QString accountId = account->uniqueIdentifier();
    mChannels.append(channel);

    // check for pending messages for this channel
    if (!mPendingMessages.contains(accountId)) {
        return;
    }

    QMap<QStringList, QStringList> &pendingMessages = mPendingMessages[accountId];
    QMap<QStringList, QStringList>::iterator it = pendingMessages.begin();
    while (it != pendingMessages.end()) {
        if (existingChat(it.key(), accountId) == channel) {
            Q_FOREACH(const QString &message, it.value()) {
                connect(channel->send(message),
                        SIGNAL(finished(Tp::PendingOperation*)),
                        SLOT(onMessageSent(Tp::PendingOperation*)));
            }
            it = pendingMessages.erase(it);
        } else {
            ++it;
        }
    }
}

void TextHandler::onMessageSent(Tp::PendingOperation *op)
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
}

Tp::TextChannelPtr TextHandler::existingChat(const QStringList &phoneNumbers, const QString &accountId)
{
    Tp::TextChannelPtr channel;

    Q_FOREACH(const Tp::TextChannelPtr &channel, mChannels) {
        int count = 0;
        Tp::AccountPtr channelAccount = TelepathyHelper::instance()->accountForConnection(channel->connection());
        if (channel->groupContacts(false).size() != phoneNumbers.size()
            || channelAccount->uniqueIdentifier() != accountId) {
            continue;
        }
        Q_FOREACH(const QString &phoneNumberNew, phoneNumbers) {
            Q_FOREACH(const Tp::ContactPtr &phoneNumberOld, channel->groupContacts(false)) {
                if (PhoneUtils::comparePhoneNumbers(phoneNumberOld->id(), phoneNumberNew)) {
                    count++;
                }
            }
        }
        if (count == phoneNumbers.size()) {
            return channel;
        }
    }
    return channel;
}

void TextHandler::onContactsAvailable(Tp::PendingOperation *op)
{
    Tp::PendingContacts *pc = qobject_cast<Tp::PendingContacts*>(op);

    if (!pc) {
        qCritical() << "The pending object is not a Tp::PendingContacts";
        return;
    }
    Tp::AccountPtr account = TelepathyHelper::instance()->accountForConnection(pc->manager()->connection());
    startChat(account, pc->contacts().toSet());
}


