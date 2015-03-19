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
#include "accountentry.h"

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
    Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->accounts()) {
        QString accountId = account->accountId();
        if (!account->connected()) {
            continue;
        }
        
        if (mPendingMessages.contains(accountId)) {
            // create text channels to send the pending messages
            Q_FOREACH(const QStringList& recipients, mPendingMessages[accountId].keys()) {
                startChat(recipients, accountId);
            }
        }
        if (mPendingMMSs.contains(accountId)) {
            // create text channels to send the pending MMSs
            Q_FOREACH(const QStringList& recipients, mPendingMMSs[accountId].keys()) {
                startChat(recipients, accountId);
            }
        }
        if (mPendingSilentMessages.contains(accountId)) {
            // create text channels to send the pending silent messages
            Q_FOREACH(const QStringList& recipients, mPendingSilentMessages[accountId].keys()) {
                startChat(recipients, accountId);
            }
        }

    }
}

TextHandler *TextHandler::instance()
{
    static TextHandler *handler = new TextHandler();
    return handler;
}

void TextHandler::startChat(const QStringList &recipients, const QString &accountId)
{
    // Request the contact to start chatting to
    // FIXME: make it possible to select which account to use, for now, pick the first one
    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
    if (!account->connected()) {
        qCritical() << "The selected account does not have a connection. AccountId:" << accountId;
        return;
    }

    connect(account->account()->connection()->contactManager()->contactsForIdentifiers(recipients),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onContactsAvailable(Tp::PendingOperation*)));
}

void TextHandler::startChatRoom(const QString &accountId, const QVariantMap &properties)
{
    Q_UNUSED(accountId)
    Q_UNUSED(properties)
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
    // FIXME: make this conditional once we add support for other IM protocols
    header["mms"] = QDBusVariant(true);
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

void TextHandler::sendMMS(const QStringList &recipients, const AttachmentList &attachments, const QString &accountId) {
    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
    if (!account) {
        // account does not exist
        return;
    }
    if (!account->connected()) {
        mPendingMMSs[accountId][recipients].append(attachments);
        return;
    }

    Tp::TextChannelPtr channel = existingChat(recipients, accountId);
    if (channel.isNull()) {
        mPendingMMSs[accountId][recipients].append(attachments);
        startChat(recipients, accountId);
        return;
    }

    Tp::MessagePartList message = buildMMS(attachments);

    connect(channel->send(message),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onMessageSent(Tp::PendingOperation*)));
}

void TextHandler::sendSilentMessage(const QStringList &recipients, const QString &message, const QString &accountId)
{
    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
    if (!account) {
        // account does not exist
        return;
    }
    if (!account->connected()) {
        mPendingSilentMessages[accountId][recipients].append(message);
        return;
    }
    Tp::TextChannelPtr channel = existingChat(recipients, accountId);
    if (channel.isNull()) {
        mPendingSilentMessages[accountId][recipients].append(message);
        startChat(recipients, accountId);
        return;
    }

    qDebug() << "TextHandler::sendSilentMessage" << message;
    Tp::MessagePart header;
    Tp::MessagePart body;
    Tp::MessagePartList fullMessage;
    header["message-type"] = QDBusVariant(0);
    header["skip-storage"] = QDBusVariant(true);
    body["content"] = QDBusVariant(message);
    body["content-type"] = QDBusVariant("text/plain");
    fullMessage << header << body;

    connect(channel->send(fullMessage),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onMessageSent(Tp::PendingOperation*)));
}

void TextHandler::sendMessage(const QStringList &recipients, const QString &message, const QString &accountId)
{
    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
    if (!account) {
        // account does not exist
        return;
    }
    if (!account->connected()) {
        mPendingMessages[accountId][recipients].append(message);
        return;
    }

    Tp::TextChannelPtr channel = existingChat(recipients, accountId);
    if (channel.isNull()) {
        mPendingMessages[accountId][recipients].append(message);
        startChat(recipients, accountId);
        return;
    }

    connect(channel->send(message),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onMessageSent(Tp::PendingOperation*)));
}

void TextHandler::acknowledgeMessages(const QStringList &recipients, const QStringList &messageIds, const QString &accountId)
{
    Tp::TextChannelPtr channel = existingChat(recipients, accountId);
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
    AccountEntry *account = TelepathyHelper::instance()->accountForConnection(channel->connection());
    if (!account) {
        return;
    }
    QString accountId = account->accountId();
    QStringList recipients;
    mChannels.append(channel);

    // check for pending messages for this channel
    if (!mPendingMessages.contains(accountId) && 
        !mPendingMMSs.contains(accountId) &&
        !mPendingSilentMessages.contains(accountId)) {
        return;
    }

    Q_FOREACH(const Tp::ContactPtr &channelContact, channel->groupContacts(false)) {
        recipients << channelContact->id();
    }

    QMap<QStringList, QStringList> &pendingMessages = mPendingMessages[accountId];
    QMap<QStringList, QStringList>::iterator it = pendingMessages.begin();
    while (it != pendingMessages.end()) {
        if (existingChat(it.key(), accountId) == channel) {
            Q_FOREACH(const QString &message, it.value()) {
                sendMessage(recipients, message, accountId);
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
                sendMMS(recipients, attachments, accountId);
            }
            it2 = pendingMMSs.erase(it2);
        } else {
            ++it2;
        }
    }
    QMap<QStringList, QStringList> &pendingSilentMessages = mPendingSilentMessages[accountId];
    QMap<QStringList, QStringList>::iterator it3 = pendingSilentMessages.begin();
    while (it3 != pendingSilentMessages.end()) {
        if (existingChat(it3.key(), accountId) == channel) {
            Q_FOREACH(const QString &message, it3.value()) {
                sendSilentMessage(recipients, message, accountId);
            }
            it3 = pendingSilentMessages.erase(it3);
        } else {
            ++it3;
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

Tp::TextChannelPtr TextHandler::existingChat(const QStringList &targetIds, const QString &accountId)
{
    Tp::TextChannelPtr channel;

    Q_FOREACH(const Tp::TextChannelPtr &channel, mChannels) {
        int count = 0;
        AccountEntry *channelAccount = TelepathyHelper::instance()->accountForConnection(channel->connection());
        if (!channelAccount || channel->groupContacts(false).size() != targetIds.size()
            || channelAccount->accountId() != accountId) {
            continue;
        }
        Q_FOREACH(const QString &targetId, targetIds) {
            Q_FOREACH(const Tp::ContactPtr &channelContact, channel->groupContacts(false)) {
                if (channelAccount->compareIds(channelContact->id(), targetId)) {
                    count++;
                }
            }
        }
        if (count == targetIds.size()) {
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
    AccountEntry *account = TelepathyHelper::instance()->accountForConnection(pc->manager()->connection());
    startChat(account->account(), pc->contacts().toSet());
}


