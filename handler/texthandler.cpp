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
#include "dbustypes.h"

#include <QImage>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>

#define SMIL_TEXT_REGION "<region id=\"Text\" width=\"100%\" height=\"100%\" fit=\"scroll\" />"
#define SMIL_IMAGE_REGION "<region id=\"Image\" width=\"100%\" height=\"100%\" fit=\"meet\" />"
#define SMIL_TEXT_PART "<par dur=\"3s\">\
       <text src=\"cid:%1\" region=\"Text\" />\
     </par>"
#define SMIL_IMAGE_PART "<par dur=\"5000ms\">\
       <img src=\"cid:%1\" region=\"Image\" />\
     </par>"

#define SMIL_FILE "<smil>\
   <head>\
     <layout>\
         %1\
     </layout>\
   </head>\
   <body>\
       %2\
   </body>\
 </smil>"

template<> bool qMapLessThanKey<QStringList>(const QStringList &key1, const QStringList &key2) 
{ 
    return key1.size() > key2.size();  // sort by operator> !
}

TextHandler::TextHandler(QObject *parent)
: QObject(parent)
{
    qDBusRegisterMetaType<AttachmentStruct>();
    qDBusRegisterMetaType<AttachmentList>();

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

Tp::MessagePartList TextHandler::buildMMS(const AttachmentList &attachments)
{
    Tp::MessagePartList message;
    Tp::MessagePart header;
    QString attachmentFilename;
    QString smil, regions, parts;
    bool hasImage = false, hasText = false;

    header["message-type"] = QDBusVariant(0);
    message << header;
    Q_FOREACH(const AttachmentStruct &attachment, attachments) {
        QByteArray fileData;
        QString newFilePath = QString(attachment.filePath).replace("file://", "");
        QFile attachmentFile(newFilePath);
        if (!attachmentFile.open(QIODevice::ReadOnly)) {
            qWarning() << "fail to load attachment" << attachmentFile.errorString() << attachment.filePath;
            continue;
        }
        if (attachment.contentType.startsWith("image/")) {
            hasImage = true;
            parts += QString(SMIL_IMAGE_PART).arg(attachment.id);
            // check if we need to reduce de image size in case it's bigger than 300k
            if (attachmentFile.size() > 307200) {
                QImage scaledImage(newFilePath);
                if (!scaledImage.isNull()) {
                    QBuffer buffer(&fileData);
                    buffer.open(QIODevice::WriteOnly);
                    scaledImage.scaled(640, 640, Qt::KeepAspectRatio).save(&buffer, "jpg");
                } else {
                    fileData = attachmentFile.readAll();
                }
            } else {
                fileData = attachmentFile.readAll();
            }
        } else if (attachment.contentType.startsWith("text/plain")) {
            hasText = true;
            parts += QString(SMIL_TEXT_PART).arg(attachment.id);
            fileData = attachmentFile.readAll();
            attachmentFile.remove();
        } else if (attachment.contentType.startsWith("text/vcard") ||
                   attachment.contentType.startsWith("text/x-vcard")) {
            fileData = attachmentFile.readAll();
        } else {
            continue;
        }

        if (hasText) {
            regions += QString(SMIL_TEXT_REGION);
        }
        if (hasImage) {
            regions += QString(SMIL_IMAGE_REGION);
        }

        Tp::MessagePart part;
        part["content-type"] =  QDBusVariant(attachment.contentType);
        part["identifier"] = QDBusVariant(attachment.id);
        part["content"] = QDBusVariant(fileData);
        part["size"] = QDBusVariant(fileData.size());
        message << part;
    }

    Tp::MessagePart smilPart;
    smil = QString(SMIL_FILE).arg(regions).arg(parts);
    smilPart["content-type"] =  QDBusVariant(QString("application/smil"));
    smilPart["identifier"] = QDBusVariant(QString("smil.xml"));
    smilPart["content"] = QDBusVariant(smil);
    smilPart["size"] = QDBusVariant(smil.size());
    message << smilPart;
    return message;
}

void TextHandler::sendMMS(const QStringList &phoneNumbers, const AttachmentList &attachments, const QString &accountId) {
   Tp::AccountPtr account = TelepathyHelper::instance()->accountForId(accountId);
    if (!TelepathyHelper::instance()->isAccountConnected(account)) {
        mPendingMMSs[accountId][phoneNumbers].append(attachments);
        return;
    }

    Tp::TextChannelPtr channel = existingChat(phoneNumbers, accountId);
    if (channel.isNull()) {
        mPendingMMSs[accountId][phoneNumbers].append(attachments);
        startChat(phoneNumbers, accountId);
        return;
    }

    Tp::MessagePartList message = buildMMS(attachments);

    connect(channel->send(message),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onMessageSent(Tp::PendingOperation*)));
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
    if (!mPendingMessages.contains(accountId) && !mPendingMMSs.contains(accountId)) {
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
    QMap<QStringList, QList<AttachmentList>> &pendingMMSs = mPendingMMSs[accountId];
    QMap<QStringList, QList<AttachmentList>>::iterator it2 = pendingMMSs.begin();
    while (it2 != pendingMMSs.end()) {
        if (existingChat(it2.key(), accountId) == channel) {
            Q_FOREACH(const AttachmentList &attachments, it2.value()) {
                connect(channel->send(buildMMS(attachments)),
                        SIGNAL(finished(Tp::PendingOperation*)),
                        SLOT(onMessageSent(Tp::PendingOperation*)));
            }
            it2 = pendingMMSs.erase(it2);
        } else {
            ++it2;
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


