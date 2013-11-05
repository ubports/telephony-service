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
    Q_FOREACH(const QString &number, mPendingMessages.keys()) {
        startChat(number);
    }
}

TextHandler *TextHandler::instance()
{
    static TextHandler *handler = new TextHandler();
    return handler;
}

void TextHandler::startChat(const QString &phoneNumber)
{
    // Request the contact to start chatting to
    Tp::AccountPtr account = TelepathyHelper::instance()->account();
    connect(account->connection()->contactManager()->contactsForIdentifiers(QStringList() << phoneNumber),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onContactsAvailable(Tp::PendingOperation*)));
}

void TextHandler::sendMessage(const QString &phoneNumber, const QString &message)
{
    if (!TelepathyHelper::instance()->connected()) {
        mPendingMessages[phoneNumber].append(message);
        return;
    }

    Tp::TextChannelPtr channel = existingChat(phoneNumber);
    if (channel.isNull()) {
        mPendingMessages[phoneNumber].append(message);
        startChat(phoneNumber);
        return;
    }

    connect(channel->send(message),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onMessageSent(Tp::PendingOperation*)));
}

void TextHandler::acknowledgeMessages(const QString &phoneNumber, const QStringList &messageIds)
{
    Tp::TextChannelPtr channel = existingChat(phoneNumber);
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
    mChannels[id] = channel;

    // check for pending messages for this channel
    QMap<QString, QStringList>::iterator it = mPendingMessages.begin();
    while (it != mPendingMessages.end()) {
        if (PhoneUtils::isSameContact(it.key(), id)) {
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

Tp::TextChannelPtr TextHandler::existingChat(const QString &phoneNumber)
{
    Tp::TextChannelPtr channel;
    QMap<QString, Tp::TextChannelPtr>::iterator it = mChannels.begin();
    while (it != mChannels.end()) {
        if (PhoneUtils::isSameContact(it.key(), phoneNumber)) {
            channel = it.value();
            break;
        }
        ++it;
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

    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    // start chatting to the contacts
    Q_FOREACH(Tp::ContactPtr contact, pc->contacts()) {
        account->ensureTextChat(contact, QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler");

        // hold the ContactPtr to make sure its refcounting stays bigger than 0
        mContacts[contact->id()] = contact;
    }
}
