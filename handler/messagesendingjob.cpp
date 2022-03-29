/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "accountentry.h"
#include "chatstartingjob.h"
#include "messagesendingjob.h"
#include "messagesendingjobadaptor.h"
#include "telepathyhelper.h"
#include "texthandler.h"
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>
#include <QImage>

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

MessageSendingJob::MessageSendingJob(TextHandler *textHandler, PendingMessage message)
: MessageJob(textHandler), mTextHandler(textHandler), mMessage(message), mFinished(false)
{
    setAdaptorAndRegister(new MessageSendingJobAdaptor(this));
}

MessageSendingJob::~MessageSendingJob()
{
    qDebug() << __PRETTY_FUNCTION__;
}

QString MessageSendingJob::accountId() const
{
    qDebug() << __PRETTY_FUNCTION__;
    return mAccountId;
}

QString MessageSendingJob::messageId() const
{
    return mMessageId;
}

QString MessageSendingJob::channelObjectPath() const
{
    qDebug() << __PRETTY_FUNCTION__;
    return mChannelObjectPath;
}

QVariantMap MessageSendingJob::properties() const
{
    return mMessage.properties;
}

void MessageSendingJob::startJob()
{
    qDebug() << __PRETTY_FUNCTION__;
    qDebug() << "Getting account for id:" << mMessage.accountId;
    AccountEntry *account = TelepathyHelper::instance()->accountForId(mMessage.accountId);
    if (!account) {
        setStatus(Failed);
        scheduleDeletion();
        return;
    }

    setStatus(Running);

    // check if the message should be sent via an overloaded account
    // if the target type is a room, do not overload.
    if (mMessage.properties["chatType"].toUInt() != Tp::HandleTypeRoom) {
        QList<AccountEntry*> overloadAccounts = TelepathyHelper::instance()->checkAccountOverload(account);
        for (auto newAccount : overloadAccounts) {
            // FIXME: check if we need to validate anything other than being connected
            if (newAccount->connected()) {
                account = newAccount;
                break;
            }
        }
    }

    // save the account
    mAccount = account;
    setAccountId(mAccount->accountId());

    if (!account->connected()) {
        connect(account, &AccountEntry::connectedChanged, [this, account]() {
            if (account->connected()) {
                findOrCreateChannel();
            }
        });
        return;
    }

    findOrCreateChannel();
}

void MessageSendingJob::findOrCreateChannel()
{
    qDebug() << __PRETTY_FUNCTION__;
    // now that we know what account to use, find existing channels or request a new one
    QList<Tp::TextChannelPtr> channels = mTextHandler->existingChannels(mAccount->accountId(), mMessage.properties);
    if (channels.isEmpty()) {
        ChatStartingJob *job = new ChatStartingJob(mTextHandler, mAccount->accountId(), mMessage.properties);
        connect(job, &MessageJob::finished, [this, job]() {
            if (job->status() == MessageJob::Failed) {
                setStatus(Failed);
                scheduleDeletion();
                return;
            }

            mTextChannel = job->textChannel();
            sendMessage();
        });
        job->startJob();
        return;
    }

    mTextChannel = channels.last();
    sendMessage();
}

void MessageSendingJob::sendMessage()
{
    qDebug() << __PRETTY_FUNCTION__;

    Tp::MessagePartList messageParts = buildMessage(mMessage);
    Tp::PendingSendMessage *op = NULL;
    // some protocols can't sent multipart messages, so we check here
    // and split the parts if needed
    if (canSendMultiPartMessages()) {
        op = mTextChannel->send(messageParts);
    } else {
        bool messageSent = false;
        Tp::MessagePart header = messageParts.takeFirst();
        Q_FOREACH(const Tp::MessagePart &part, messageParts) {
            Tp::MessagePartList newMessage;
            newMessage << header;
            newMessage << part;
            Tp::PendingSendMessage *thisOp = mTextChannel->send(newMessage);
            if (messageSent) {
                continue;
            }
            messageSent = true;
            op = thisOp;
        }
    }
    connect(op, &Tp::PendingOperation::finished, [this, op]() {
        if (op->isError()) {
            setStatus(Failed);
            scheduleDeletion();
            return;
        }

        setChannelObjectPath(mTextChannel->objectPath());
        setMessageId(op->sentMessageToken());
        setStatus(Finished);
        scheduleDeletion();
    });
}
 
bool MessageSendingJob::canSendMultiPartMessages()
{
    if (!mAccount) {
        return false;
    }
    switch (mAccount->type()) {
    case AccountEntry::PhoneAccount:
        return true;
    // TODO check in telepathy if multipart is supported
    // currently we just return false to be on the safe side
    case AccountEntry::GenericAccount:
    default:
        return false;
    }
    return false;
}

void MessageSendingJob::setAccountId(const QString &accountId)
{
    qDebug() << __PRETTY_FUNCTION__;
    mAccountId = accountId;
    Q_EMIT accountIdChanged();
}

void MessageSendingJob::setChannelObjectPath(const QString &objectPath)
{
    qDebug() << __PRETTY_FUNCTION__;
    mChannelObjectPath = objectPath;
    Q_EMIT channelObjectPathChanged();
}

void MessageSendingJob::setMessageId(const QString &id)
{
    mMessageId = id;
    Q_EMIT messageIdChanged();
}

Tp::MessagePartList MessageSendingJob::buildMessage(const PendingMessage &pendingMessage)
{
    qDebug() << __PRETTY_FUNCTION__;
    Tp::MessagePartList message;
    Tp::MessagePart header;
    QString smil, regions, parts;
    bool hasImage = false, hasText = false, hasVideo = false, hasAudio = false, isMMS = false;
    int chatType = pendingMessage.properties["chatType"].toUInt();

    if (!mAccount) {
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
    if (mAccount->type() == AccountEntry::PhoneAccount) {
        isMMS = (pendingMessage.attachments.size() > 0 ||
                 (pendingMessage.properties["chatType"].toUInt() == Tp::HandleTypeRoom));
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
        QString contentType = attachment.contentType;
        QString filename = attachment.id;
        QString newFilePath = QString(attachment.filePath).replace("file://", "");
        QFile attachmentFile(newFilePath);
        if (!attachmentFile.open(QIODevice::ReadOnly)) {
            qWarning() << "fail to load attachment" << attachmentFile.errorString() << attachment.filePath;
            continue;
        }
        if (attachment.contentType.startsWith("image/")) {
            if (isMMS) {
                hasImage = true;
                // check if we need to reduce de image size in case it's bigger than 300k
                // this check is only valid for MMS
                if (attachmentFile.size() > 307200) {
                    QImage scaledImage(newFilePath);
                    if (!scaledImage.isNull()) {
                        QBuffer buffer(&fileData);
                        buffer.open(QIODevice::WriteOnly);
                        scaledImage.scaled(640, 640, Qt::KeepAspectRatio, Qt::SmoothTransformation).save(&buffer, "jpg");
                        // update metadatas
                        QFileInfo info(newFilePath);
                        filename = info.completeBaseName() + ".jpg";
                        contentType = "image/jpeg";
                    }
                } else {
                    fileData = attachmentFile.readAll();
                }
                parts += QString(SMIL_IMAGE_PART).arg(filename);

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
        part["content-type"] =  QDBusVariant(contentType);
        part["identifier"] = QDBusVariant(filename);
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


