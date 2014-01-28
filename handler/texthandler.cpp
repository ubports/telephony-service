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
    if (!TelepathyHelper::instance()->account() || !TelepathyHelper::instance()->account()->connection()) {
        return;
    }

    // create text channels to send the pending messages
    Q_FOREACH(const QStringList& phoneNumbers, mPendingMessages.keys()) {
        startChat(phoneNumbers);
    }
}

TextHandler *TextHandler::instance()
{
    static TextHandler *handler = new TextHandler();
    return handler;
}

void TextHandler::startChat(const QStringList &phoneNumbers)
{
    // Request the contact to start chatting to
    Tp::AccountPtr account = TelepathyHelper::instance()->account();
    connect(account->connection()->contactManager()->contactsForIdentifiers(phoneNumbers),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onContactsAvailable(Tp::PendingOperation*)));
}

void TextHandler::startChat(const Tp::Contacts &contacts)
{
    qDebug() << "TextHandler::startChat" << contacts;
    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    Tp::ContactPtr firstContact = contacts.values()[0];
    account->ensureTextChat(firstContact, QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler");
    // start chatting to the contacts
    Q_FOREACH(Tp::ContactPtr contact, contacts) {
        // hold the ContactPtr to make sure its refcounting stays bigger than 0
        mContacts[contact->id()] = contact;
    }
    mPendingMembers[firstContact->id()] = contacts;
}

void TextHandler::sendMessage(const QStringList &phoneNumbers, const QString &message)
{
    if (!TelepathyHelper::instance()->connected()) {
        mPendingMessages[phoneNumbers].append(message);
        return;
    }

    Tp::TextChannelPtr channel = existingChat(phoneNumbers);
    if (channel.isNull()) {
        mPendingMessages[phoneNumbers].append(message);
        startChat(phoneNumbers);
        return;
    }

    connect(channel->send(message),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onMessageSent(Tp::PendingOperation*)));
}

void TextHandler::acknowledgeMessages(const QStringList &phoneNumbers, const QStringList &messageIds)
{
    Tp::TextChannelPtr channel = existingChat(phoneNumbers);
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
    QString id = channel->targetContact()->id();

    mChannels.append(channel);

    connect(channel->groupAddContacts(mPendingMembers[id].toList()), 
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(processPendingMessages(Tp::PendingOperation*)));
    mPendingMembers.remove(id);
}

void TextHandler::processPendingMessages(Tp::PendingOperation *op)
{
    // check for pending messages for this channel
    Q_FOREACH(const Tp::TextChannelPtr &channel, mChannels) {
        QMap<QStringList, QStringList>::iterator it = mPendingMessages.begin();
        while (it != mPendingMessages.end()) {
            if (existingChat(it.key()) == channel) {
                Q_FOREACH(const QString &message, it.value()) {
                    connect(channel->send(message),
                            SIGNAL(finished(Tp::PendingOperation*)),
                            SLOT(onMessageSent(Tp::PendingOperation*)));
                }
                it = mPendingMessages.erase(it);
            } else {
                ++it;
            }
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

Tp::TextChannelPtr TextHandler::existingChat(const QStringList &phoneNumbers)
{
    Tp::TextChannelPtr channel;

    QList<Tp::TextChannelPtr>::iterator it = mChannels.begin();
    Q_FOREACH(const Tp::TextChannelPtr &channel, mChannels) {
        int count = 0;
        qDebug() << phoneNumbers.size() << channel->groupContacts(false).size();
        if (channel->groupContacts(false).size() != phoneNumbers.size()) {
            continue;
        }
        Q_FOREACH(const QString &phoneNumberNew, phoneNumbers) {
            Q_FOREACH(const Tp::ContactPtr &phoneNumberOld, channel->groupContacts(false)) {
                if (PhoneUtils::comparePhoneNumbers(phoneNumberOld->id(), phoneNumberNew)) {
                    count++;
                }
            }
        }
        qDebug() << count << phoneNumbers;
        if (count == phoneNumbers.size()) {
            qDebug() << "channel found";
            return channel;
        }
    }
    qDebug() << "channel not found" << channel;
    return channel;
}

void TextHandler::onContactsAvailable(Tp::PendingOperation *op)
{
    Tp::PendingContacts *pc = qobject_cast<Tp::PendingContacts*>(op);

    if (!pc) {
        qCritical() << "The pending object is not a Tp::PendingContacts";
        return;
    }
    startChat(pc->contacts().toSet());
}


