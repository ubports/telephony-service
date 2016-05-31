/*
 * Copyright (C) 2012-2015 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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

#include "texthandler.h"
#include "phoneutils.h"
#include "telepathyhelper.h"
#include "config.h"
#include "dbustypes.h"
#include "accountentry.h"

#include <QImage>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>
#include <TelepathyQt/PendingChannelRequest>

#define SMIL_TEXT_REGION "<region id=\"Text\" width=\"100%\" height=\"100%\" fit=\"scroll\" />"
#define SMIL_IMAGE_REGION "<region id=\"Image\" width=\"100%\" height=\"100%\" fit=\"meet\" />"
#define SMIL_VIDEO_REGION "<region id=\"Video\" width=\"100%\" height=\"100%\" fit=\"meet\" />"
#define SMIL_AUDIO_REGION "<region id=\"Audio\" width=\"100%\" height=\"100%\" fit=\"meet\" />"
#define SMIL_TEXT_PART "<par dur=\"3s\">\
       <text src=\"cid:%1\" region=\"Text\" />\
     </par>"
#define SMIL_IMAGE_PART "<par dur=\"5000ms\">\
       <img src=\"cid:%1\" region=\"Image\" />\
     </par>"
#define SMIL_VIDEO_PART "<par>\
       <video src=\"cid:%1\" region=\"Video\" />\
     </par>"
#define SMIL_AUDIO_PART "<par>\
       <audio src=\"cid:%1\" region=\"Audio\" />\
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
    qRegisterMetaType<PendingMessage>();

    // track when the account becomes available
    connect(TelepathyHelper::instance(),
            SIGNAL(setupReady()),
            SLOT(onConnectedChanged()));
}

void TextHandler::onConnectedChanged()
{
    if (!TelepathyHelper::instance()->ready()) {
        return;
    }

    // now check which accounts are connected
    Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->accounts()) {
        QString accountId = account->accountId();
        if (!account->connected()) {
            continue;
        }
        
        // create text channels to send the pending messages
        QList<QStringList> recipientsList;
        Q_FOREACH(const PendingMessage &pendingMessage, mPendingMessages) {
            if (accountId != pendingMessage.accountId) {
                continue;
            }
            bool found = false;
            // avoid adding twice the same list of participants
/*            Q_FOREACH(const QStringList &recipients, recipientsList) {
                if (recipients == pendingMessage.recipients) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                recipientsList << pendingMessage.recipients;
            }*/
            // TODO AVOID CALLING TWICE FOR SAME CHANNEL
            startChat(accountId, pendingMessage.properties);
        }
    }
}

TextHandler *TextHandler::instance()
{
    static TextHandler *handler = new TextHandler();
    return handler;
}

void TextHandler::startChat(const QString &accountId, const QVariantMap &properties)
{
    // Request the contact to start chatting to
    // FIXME: make it possible to select which account to use, for now, pick the first one
    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
    if (!account || !account->connected()) {
        qCritical() << "The selected account does not have a connection. AccountId:" << accountId;
        return;
    }

    switch(properties["chatType"].toUInt()) {
    case Tp::HandleTypeNone:
    case Tp::HandleTypeContact:
        startTextChat(account->account(), properties);
        break;
    case Tp::HandleTypeRoom:
        startTextChatroom(account->account(), properties);
        break;
    default:
        qCritical() << "Chat type not supported";
    }
}

void TextHandler::startTextChat(const Tp::AccountPtr &account, const QVariantMap &properties)
{
    QStringList participants = properties["participantIds"].toStringList();
    switch(participants.size()) {
    case 0:
        qCritical() << "Error: No participant list provided";
        break;
    case 1:
        account->ensureTextChat(participants[0], QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler");
        break;
    default:
        account->createConferenceTextChat(QList<Tp::ChannelPtr>(), participants, QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler");
    }
}

Tp::TextChannelPtr TextHandler::startTextChatroom(const Tp::AccountPtr &account, const QVariantMap &properties)
{
    QString roomName = properties["threadId"].toString();

    // these properties are still not used
    //QString server = properties["Server"].toString();
    //QString creator = properties["Creator"].toString();

    QVariantMap request;
    Tp::PendingChannelRequest *op = NULL;
    if (roomName.isEmpty()) {
        request.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".ChannelType"), TP_QT_IFACE_CHANNEL_TYPE_TEXT);
        request.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandleType"), (uint) Tp::HandleTypeNone);
        QStringList initialInviteeIDs = properties["participantIds"].toStringList();
        if (!initialInviteeIDs.isEmpty()) {
            request.insert(TP_QT_IFACE_CHANNEL_INTERFACE_CONFERENCE + QLatin1String(".InitialInviteeIDs"), initialInviteeIDs);
        }
        // the presence of RoomName indicates the returned channel must be of type Room
        request.insert(TP_QT_IFACE_CHANNEL_INTERFACE_ROOM + QLatin1String(".RoomName"), QString());

        // TODO use the instance returned by createChanne() to track when the channel creation is finished
        op = account->createChannel(request, QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler");
    } else {

        op = account->ensureTextChatroom(roomName, QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler", request);
    }

    if (!op) { 
        return Tp::TextChannelPtr();
    }
    while (!op->isFinished()) {
        qApp->processEvents();
    }
    Tp::TextChannelPtr textChannel(qobject_cast<Tp::TextChannel*>(op->channelRequest()->channel().data()));
    return textChannel;
}

Tp::MessagePartList TextHandler::buildMessage(const PendingMessage &pendingMessage)
{
    Tp::MessagePartList message;
    Tp::MessagePart header;
    QString smil, regions, parts;
    bool hasImage = false, hasText = false, hasVideo = false, hasAudio = false, isMMS = false;

    AccountEntry *account = TelepathyHelper::instance()->accountForId(pendingMessage.accountId);
    if (!account) {
        // account does not exist
        return Tp::MessagePartList();
    }

    bool temporaryFiles = (pendingMessage.properties.contains("x-canonical-tmp-files") &&
                           pendingMessage.properties["x-canonical-tmp-files"].toBool());

    // add the remaining properties to the message header
    QVariantMap::const_iterator it = pendingMessage.properties.begin();
    for (; it != pendingMessage.properties.end(); ++it) {
        header[it.key()] = QDBusVariant(it.value());
    }

    // check if this message should be sent as an MMS
    if (account->type() == AccountEntry::PhoneAccount) {
        isMMS = (pendingMessage.attachments.size() > 0 ||
                 (header.contains("x-canonical-mms") && header["x-canonical-mms"].variant().toBool()) ||
                 (pendingMessage.properties["participantIds"].toStringList().size() > 1 && TelepathyHelper::instance()->mmsGroupChat()));
        if (isMMS) {
            header["x-canonical-mms"] = QDBusVariant(true);
        }
    }

    // this flag should not be in the message header, it's only useful for the handler
    header.remove("x-canonical-tmp-files");
    header.remove("chatType");
    header.remove("threadId");
    header.remove("participantIds");

    header["message-type"] = QDBusVariant(0);
    message << header;

    // convert AttachmentList struct into telepathy Message parts
    Q_FOREACH(const AttachmentStruct &attachment, pendingMessage.attachments) {
        QByteArray fileData;
        QString newFilePath = QString(attachment.filePath).replace("file://", "");
        QFile attachmentFile(newFilePath);
        if (!attachmentFile.open(QIODevice::ReadOnly)) {
            qWarning() << "fail to load attachment" << attachmentFile.errorString() << attachment.filePath;
            continue;
        }
        if (attachment.contentType.startsWith("image/")) {
            if (isMMS) {
                hasImage = true;
                parts += QString(SMIL_IMAGE_PART).arg(attachment.id);
                // check if we need to reduce de image size in case it's bigger than 300k
                // this check is only valid for MMS
                if (attachmentFile.size() > 307200) {
                    QImage scaledImage(newFilePath);
                    if (!scaledImage.isNull()) {
                        QBuffer buffer(&fileData);
                        buffer.open(QIODevice::WriteOnly);
                        scaledImage.scaled(640, 640, Qt::KeepAspectRatio, Qt::SmoothTransformation).save(&buffer, "jpg");
                    }
                } else {
                    fileData = attachmentFile.readAll();
                }
            }
        } else if (attachment.contentType.startsWith("video/")) {
            if (isMMS) {
                hasVideo = true;
                parts += QString(SMIL_VIDEO_PART).arg(attachment.id);
            }
        } else if (attachment.contentType.startsWith("audio/")) {
            if (isMMS) {
                hasAudio = true;
                parts += QString(SMIL_AUDIO_PART).arg(attachment.id);
            }
        } else if (attachment.contentType.startsWith("text/plain")) {
            if (isMMS) {
                hasText = true;
                parts += QString(SMIL_TEXT_PART).arg(attachment.id);
            }
        } else if (attachment.contentType.startsWith("text/vcard") ||
                   attachment.contentType.startsWith("text/x-vcard")) {
        } else if (isMMS) {
            // for MMS we just support the contentTypes above
            if (temporaryFiles) {
                attachmentFile.remove();
            }
            continue;
        }

        if (fileData.isEmpty()) {
            fileData = attachmentFile.readAll();
        }

        if (temporaryFiles) {
            attachmentFile.remove();
        }

        if (hasVideo) {
            regions += QString(SMIL_VIDEO_REGION);
        }

        if (hasAudio) {
            regions += QString(SMIL_AUDIO_REGION);
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

    if (!pendingMessage.message.isEmpty()) {
        Tp::MessagePart part;
        QString tmpTextId("text_0.txt");
        part["content-type"] =  QDBusVariant(QString("text/plain"));
        part["identifier"] = QDBusVariant(tmpTextId);
        part["content"] = QDBusVariant(pendingMessage.message);
        part["size"] = QDBusVariant(pendingMessage.message.size());
        if (isMMS) {
            parts += QString(SMIL_TEXT_PART).arg(tmpTextId);
            regions += QString(SMIL_TEXT_REGION);
        }
        message << part;
    }

    if (isMMS) {
        Tp::MessagePart smilPart;
        smil = QString(SMIL_FILE).arg(regions).arg(parts);
        smilPart["content-type"] =  QDBusVariant(QString("application/smil"));
        smilPart["identifier"] = QDBusVariant(QString("smil.xml"));
        smilPart["content"] = QDBusVariant(smil);
        smilPart["size"] = QDBusVariant(smil.size());
 
        message << smilPart;
    }

    return message;
}

QString TextHandler::sendMessage(const QString &accountId, const QString &message, const AttachmentList &attachments, const QVariantMap &properties)
{
    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
    if (!account) {
        // account does not exist
        return QString();
    }

    // check if the message should be sent via multimedia account
    // we just use fallback to 1-1 chats
    if (account->type() == AccountEntry::PhoneAccount) {
        Q_FOREACH(AccountEntry *newAccount, TelepathyHelper::instance()->accounts()) {
            // TODO: we have to find the multimedia account that matches the same phone number, 
            // but for now we just pick any multimedia connected account
            if (newAccount->type() != AccountEntry::MultimediaAccount) {
                continue;
            }
            // FIXME: the fallback implementation needs to be changed to use protocol info and create a map of
            // accounts. Also, it needs to check connection capabilities to determine if we can send message
            // to offline contacts.
            bool shouldFallback = true;
            // if the account is offline, dont fallback to this account
            if (!newAccount->connected()) {
                continue;
            }
            QList<Tp::TextChannelPtr> channels = existingChannels(newAccount->accountId(), properties);
            // check if we have a channel for this contact already and get the contact pointer from there,
            // this way we avoid doing the while(op->isFinished()) all the time
            if (!channels.isEmpty()) {
                // if the contact is known, force fallback to this account
                Q_FOREACH(const Tp::ContactPtr &contact, channels.first()->groupContacts(false)) {
                    Tp::Presence presence = contact->presence();
                    shouldFallback = (presence.type() == Tp::ConnectionPresenceTypeAvailable ||
                                      presence.type() == Tp::ConnectionPresenceTypeOffline);
                    if (!shouldFallback) {
                        break;
                    }
                }
            } else {
                QStringList participants = properties["participantIds"].toStringList();
                Tp::PendingOperation *op = newAccount->account()->connection()->contactManager()->contactsForIdentifiers(participants);
                while (!op->isFinished()) {
                    qApp->processEvents();
                }
                Tp::PendingContacts *pc = qobject_cast<Tp::PendingContacts*>(op);
                if (pc) {
                    Q_FOREACH(const Tp::ContactPtr &contact, pc->contacts()) {
                        Tp::Presence presence = contact->presence();
                        shouldFallback = (presence.type() == Tp::ConnectionPresenceTypeAvailable ||
                                          presence.type() == Tp::ConnectionPresenceTypeOffline);
                        if (!shouldFallback) {
                            break;
                        }
                    }
                }
            }
            if (shouldFallback) {
                account = newAccount;
                break;
            }
        }
    }

    // keep recipient list always sorted to be able to compare
    PendingMessage pendingMessage = {account->accountId(), message, attachments, properties};

    if (!account->connected()) {
        mPendingMessages.append(pendingMessage);
        return account->accountId();
    }

    QList<Tp::TextChannelPtr> channels = existingChannels(account->accountId(), properties);
    if (channels.isEmpty()) {
        // temporary
        switch(properties["chatType"].toUInt()) {
        case Tp::HandleTypeNone:
        case Tp::HandleTypeContact:
            mPendingMessages.append(pendingMessage);
            startTextChat(account->account(), pendingMessage.properties);
            return account->accountId();
        case Tp::HandleTypeRoom: {
            channels << startTextChatroom(account->account(), pendingMessage.properties);
            qDebug() << "channel returned" << channels.last();
       
            // multimedia fails if we send the message right away
            QTimer *timer = new QTimer(this);
            timer->setInterval(3000);
            timer->setSingleShot(true);
            QObject::connect(timer, &QTimer::timeout, [=]() {
                qDebug() << "sending message" << channels.last();
                connect(channels.last()->send(buildMessage(pendingMessage)),
                        SIGNAL(finished(Tp::PendingOperation*)),
                        SLOT(onMessageSent(Tp::PendingOperation*)));

                timer->deleteLater();
            });
            timer->start();
            return account->accountId();
            break;
        }
        }

        //startChat(account->accountId(), pendingMessage.properties);
        //return account->accountId();
    }

    connect(channels.last()->send(buildMessage(pendingMessage)),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onMessageSent(Tp::PendingOperation*)));

    return account->accountId();
}

void TextHandler::acknowledgeMessages(const QVariantList &messages)
{
    Q_FOREACH(const QVariant &message, messages) {
        QVariantMap properties = qdbus_cast<QVariantMap>(message);
        QList<Tp::TextChannelPtr> channels = existingChannels(properties["accountId"].toString(), properties);
        if (channels.isEmpty()) {
            return;
        }

        QList<Tp::ReceivedMessage> messagesToAck;
        QString messageId = properties["messageId"].toString();
        Q_FOREACH(const Tp::TextChannelPtr &channel, channels) {
            Q_FOREACH(const Tp::ReceivedMessage &message, channel->messageQueue()) {
                if (messageId == message.messageToken()) {
                    messagesToAck.append(message);
                }
            }
            channel->acknowledge(messagesToAck);
        }
    }
}

void TextHandler::acknowledgeAllMessages(const QVariantMap &properties)
{
    QList<Tp::TextChannelPtr> channels = existingChannels(properties["accountId"].toString(), properties);
    if (channels.isEmpty()) {
        return;
    }

    Q_FOREACH(const Tp::TextChannelPtr &channel, channels) {
        channel->acknowledge(channel->messageQueue());
    }
}

void TextHandler::onTextChannelInvalidated()
{
    Tp::TextChannelPtr textChannel(qobject_cast<Tp::TextChannel*>(sender()));
    mChannels.removeAll(textChannel);
}

void TextHandler::onTextChannelAvailable(Tp::TextChannelPtr channel)
{
    qDebug() << "TextHandler::onTextChannelAvailable" << channel;
    AccountEntry *account = TelepathyHelper::instance()->accountForConnection(channel->connection());
    if (!account) {
        return;
    }
    connect(channel.data(),
            SIGNAL(invalidated(Tp::DBusProxy*,const QString&, const QString&)),
            SLOT(onTextChannelInvalidated()));

    QString accountId = account->accountId();
    mChannels.append(channel);

    // check for pending messages for this channel
    if (mPendingMessages.isEmpty()) {
        return;
    }

    QList<PendingMessage>::iterator it = mPendingMessages.begin();
    while (it != mPendingMessages.end()) {
        bool found = false;
        Q_FOREACH(const Tp::TextChannelPtr &existingChannel, existingChannels(it->accountId, it->properties)) {
            if (existingChannel == channel) {
                sendMessage(it->accountId, it->message, it->attachments, it->properties);
                it = mPendingMessages.erase(it);
                found = true;
                break;
            }
        }
        if (!found) {
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

QList<Tp::TextChannelPtr> TextHandler::existingChannels(const QString &accountId, const QVariantMap &properties)
{
    QList<Tp::TextChannelPtr> channels;
    QStringList targetIds = properties["participantIds"].toStringList();
    int chatType = properties["chatType"].toUInt();
    if (chatType == 0 && targetIds.size() == 1) {
        chatType = 1;
    }
    QString roomId = properties["threadId"].toString();

    Q_FOREACH(const Tp::TextChannelPtr &channel, mChannels) {
        int count = 0;
        AccountEntry *channelAccount = TelepathyHelper::instance()->accountForConnection(channel->connection());

        if (!channelAccount || channelAccount->accountId() != accountId) {
            continue;
        }

        if (chatType != channel->targetHandleType()) {
            continue;
        }

        if (chatType == 2) {
            if (!roomId.isEmpty() && channel->targetHandleType() == chatType && roomId == channel->targetId()) {
                channels.append(channel);
            }
            continue;
        }

        // this is a special case. We have to check if we are looking for a channel open with our self contact.
        bool channelToSelfContact = channel->groupContacts(true).size() == 1 && targetIds.size() == 1 &&
                          channel->targetHandleType() == Tp::HandleTypeContact &&
                          channelAccount->compareIds(channel->targetId(), channelAccount->selfContactId()) &&
                          channelAccount->compareIds(targetIds.first(), channel->targetId()); // make sure we compare the recipient with the channel

        if (channelToSelfContact) {
            channels.append(channel);
            continue;
        }

        if (channel->groupContacts(false).size() != targetIds.size()) {
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
            channels.append(channel);
        }
    }
    return channels;
}
