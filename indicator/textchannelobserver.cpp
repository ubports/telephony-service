/*
 * Copyright (C) 2012-2014 Canonical, Ltd.
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

#include "applicationutils.h"
#include "greetercontacts.h"
#include "textchannelobserver.h"
#include "messagingmenu.h"
#include "metrics.h"
#include "chatmanager.h"
#include "callmanager.h"
#include "config.h"
#include "contactutils.h"
#include "ringtone.h"
#include "telepathyhelper.h"
#include "phoneutils.h"
#include "accountentry.h"
#include "ofonoaccountentry.h"
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>
#include <TelepathyQt/ReferencedHandles>
#include <QContactAvatar>
#include <QContactDisplayLabel>
#include <QContactFetchRequest>
#include <QContactFilter>
#include <QContactPhoneNumber>
#include <QImage>
#include <History/TextEvent>
#include <History/Manager>

namespace C {
#include <libintl.h>
}

// notification handling

class NotificationData {
public:
    QString senderId;
    QStringList participantIds;
    QString accountId;
    QDateTime timestamp;
    QString text;
    QString eventId;
    QString alias;
    QString message;
    TextChannelObserver *observer;
    QMap<NotifyNotification*, NotificationData*> *notificationList;
};

void notification_closed(NotifyNotification *notification, QMap<NotifyNotification*, NotificationData*> *map);

void sim_selection_action(NotifyNotification* notification, char *action, gpointer data)
{
    GError *error = NULL;
    QString accountId = action;
    NotificationData *notificationData = (NotificationData*) data;
    if (notificationData != NULL) {
        QStringList recipients;
        recipients << notificationData->participantIds;
        recipients.removeDuplicates();
        notificationData->observer->sendMessage(recipients, notificationData->text, accountId);
    }

    notify_notification_close(notification, &error);
    g_object_unref(notification);
}

void flash_notification_action(NotifyNotification* notification, char *action, gpointer data)
{
    GError *error = NULL;
    if (action == QLatin1String("notification_save_action")) {
        NotificationData *notificationData = (NotificationData*) data;
        if (notificationData != NULL) {
            AccountEntry *account = TelepathyHelper::instance()->accountForId(notificationData->accountId);
            bool phoneNumberBased = account && (account->type() == AccountEntry::PhoneAccount || account->type() == AccountEntry::MultimediaAccount);
            QStringList recipients;
            recipients << notificationData->senderId << notificationData->participantIds;
            History::Thread thread = History::Manager::instance()->threadForParticipants(notificationData->accountId,
                                                                                         History::EventTypeText,
                                                                                         recipients,
                                                                                         phoneNumberBased ? History::MatchPhoneNumber :
                                                                                                            History::MatchCaseSensitive,
                                                                                         true);
            History::TextEvent textEvent(notificationData->accountId,
                                         thread.threadId(), 
                                         notificationData->eventId, 
                                         notificationData->senderId,
                                         notificationData->timestamp,
                                         false,
                                         notificationData->text,
                                         History::MessageTypeText);
            History::Events events;
            events.append(textEvent);

            History::Manager::instance()->writeEvents(events);
        }
    }
    notify_notification_close(notification, &error);

    g_object_unref(notification);
}


void notification_action(NotifyNotification* notification, char *action, gpointer data)
{
    Q_UNUSED(notification);
    Q_UNUSED(action);

    NotificationData *notificationData = (NotificationData*) data;
    if (notificationData != NULL) {
        // launch the messaging-app to show the message
        QStringList recipients;
        QString accountId = notificationData->accountId;
        if (!notificationData->senderId.isEmpty()) {
            recipients << notificationData->senderId;
        }
        recipients << notificationData->participantIds;
        recipients.removeDuplicates();
 
        QString url(QString("message:///%1").arg(QString(QUrl::toPercentEncoding(recipients.join(";")))));
        AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
        if (account && account->type() == AccountEntry::GenericAccount) {
            url += QString("?accountId=%1").arg(QString(QUrl::toPercentEncoding(accountId)));
        }
  
        ApplicationUtils::openUrl(url);

        notification_closed(notification, notificationData->notificationList);
    }
    g_object_unref(notification);
}

void notification_closed(NotifyNotification *notification, QMap<NotifyNotification*, NotificationData*> *map)
{
    NotificationData *data = map->take(notification);
    if (data != NULL) {
        delete data;
    }
}

TextChannelObserver::TextChannelObserver(QObject *parent) :
    QObject(parent)
{
    connect(MessagingMenu::instance(),
            SIGNAL(replyReceived(QStringList,QString,QString)),
            SLOT(onReplyReceived(QStringList,QString,QString)));
    connect(MessagingMenu::instance(),
            SIGNAL(messageRead(QStringList,QString,QString)),
            SLOT(onMessageRead(QStringList,QString,QString)));

    if (GreeterContacts::isGreeterMode()) {
        connect(GreeterContacts::instance(), SIGNAL(contactUpdated(QtContacts::QContact)),
                this, SLOT(updateNotifications(QtContacts::QContact)));
    }
}

TextChannelObserver::~TextChannelObserver()
{
    QMap<NotifyNotification*, NotificationData*>::const_iterator i = mNotifications.constBegin();
    while (i != mNotifications.constEnd()) {
        NotifyNotification *notification = i.key();
        NotificationData *data = i.value();
        g_signal_handlers_disconnect_by_data(notification, &mNotifications);
        delete data;
        ++i;
    }
    mNotifications.clear();
}

void TextChannelObserver::sendMessage(const QStringList &recipients, const QString &text, const QString &accountId)
{
    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
    if (!account || accountId.isEmpty()) {
        // fallback to the default account
        if (TelepathyHelper::instance()->defaultMessagingAccount() && TelepathyHelper::instance()->defaultMessagingAccount()->active()) {
            account = TelepathyHelper::instance()->defaultMessagingAccount();
        } else if (TelepathyHelper::instance()->activeAccounts().size() > 0) {
            account = TelepathyHelper::instance()->activeAccounts()[0];
        }
    }

    if (!account) {
        // we could not find any account, but in theory this case can never happen
        return;
    }

    // check if the account is available
    if (!account->connected()) {
        bool phoneNumberBased = account->type() == AccountEntry::PhoneAccount || account->type() == AccountEntry::MultimediaAccount;
        History::Thread thread = History::Manager::instance()->threadForParticipants(account->accountId(),
                                                                                     History::EventTypeText,
                                                                                     recipients,
                                                                                     phoneNumberBased ? History::MatchPhoneNumber :
                                                                                                        History::MatchCaseSensitive,
                                                                                     true);
        History::TextEvent textEvent(account->accountId(),
                                     thread.threadId(),
                                     QByteArray(QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz").toUtf8()).toHex(),
                                     "self",
                                     QDateTime::currentDateTime(),
                                     false,
                                     text,
                                     History::MessageTypeText,
                                     History::MessageStatusPermanentlyFailed);
        History::Events events;
        events.append(textEvent);

        History::Manager::instance()->writeEvents(events);

        QString failureMessage;
        OfonoAccountEntry *ofonoAccount = qobject_cast<OfonoAccountEntry*>(account);
        bool simLocked = (ofonoAccount && ofonoAccount->simLocked());

        if (simLocked) {
            failureMessage = C::gettext("Unlock your sim card and try again from the messaging application.");
        } else if (ofonoAccount && TelepathyHelper::instance()->flightMode()) {
            failureMessage = C::gettext("Deactivate flight mode and try again from the messaging application.");
        } else {
            // generic error
            failureMessage = C::gettext("Try again from the messaging application.");
        }

        // notify user about the failure
        GIcon *icon = g_themed_icon_new("cancel");
        gchar *iconPath = g_icon_to_string(icon);
        NotifyNotification *notification = notify_notification_new(C::gettext("The message could not be sent"),
                                                               failureMessage.toStdString().c_str(),
                                                               iconPath);
        g_object_unref(icon);
        g_free(iconPath);
        NotificationData *data = new NotificationData();
        data->participantIds = recipients;
        data->accountId = account->accountId();
        data->message = text;
        data->notificationList = &mNotifications;
        mNotifications.insert(notification, data);

        // add the callback action
        notify_notification_add_action (notification,
                                        "notification_action",
                                        C::gettext("View message"),
                                        notification_action,
                                        data,
                                        NULL /* will be deleted when closed */);

        notify_notification_set_hint_string(notification,
                                            "x-canonical-switch-to-application",
                                            "true");

        g_signal_connect(notification, "closed", G_CALLBACK(notification_closed), &mNotifications);

        GError *error = NULL;
        if (!notify_notification_show(notification, &error)) {
            qWarning() << "Failed to show message notification:" << error->message;
            g_error_free (error);
        }

        return;
    }

    ChatManager::instance()->sendMessage(account->accountId(), recipients, text);
}

void TextChannelObserver::clearNotifications()
{
    Q_FOREACH(NotifyNotification *notification, mNotifications.keys()) {
        GError *error = NULL;
        notify_notification_close(notification, &error);
    }
}

void TextChannelObserver::showNotificationForFlashMessage(const Tp::ReceivedMessage &message, const QString &accountId)
{
    Tp::ContactPtr contact = message.sender();
    QByteArray token(message.messageToken().toUtf8());
    MessagingMenu::instance()->addFlashMessage(contact->id(), accountId, token.toHex(), message.received(), message.text());


    // show the notification
    NotifyNotification *notification = notify_notification_new("",
                                                               message.text().toStdString().c_str(),
                                                               "");
    NotificationData *data = new NotificationData();
    data->senderId = contact->id();
    data->accountId = accountId;
    data->timestamp = message.received();
    data->text = message.text();
    data->eventId = message.messageToken().toUtf8();
    mNotifications.insert(notification, data);
 
    notify_notification_add_action (notification,
                                    "notification_ok_action",
                                    C::gettext("Ok"),
                                    flash_notification_action,
                                    NULL,
                                    NULL);
    notify_notification_add_action (notification,
                                    "notification_save_action",
                                    C::gettext("Save"),
                                    flash_notification_action,
                                    data,
                                    NULL);
    g_signal_connect(notification, "closed", G_CALLBACK(notification_closed), &mNotifications);

    notify_notification_set_hint_string(notification,
                                        "x-canonical-snap-decisions",
                                        "true");
    notify_notification_set_hint_string(notification,
                                        "x-canonical-private-button-tint",
                                        "true");


    GError *error = NULL;
    if (!notify_notification_show(notification, &error)) {
        qWarning() << "Failed to show message notification:" << error->message;
        g_error_free (error);
    }

    Ringtone::instance()->playIncomingMessageSound();
}

void TextChannelObserver::triggerNotificationForMessage(const Tp::ReceivedMessage &message, const QString &accountId, const QStringList &participantIds)
{
    Tp::ContactPtr contact = message.sender();

    QByteArray token(message.messageToken().toUtf8());
    if (!mUnreadMessages.contains(token)) {
        Ringtone::instance()->playIncomingMessageSound();
        return;
    }

    if (GreeterContacts::isGreeterMode()) { // we're in the greeter's session
        GreeterContacts::instance()->setContactFilter(QContactPhoneNumber::match(contact->id()));
        // in greeter mode we show the notification right away as the contact data might not be received
        showNotificationForMessage(message, accountId, participantIds);
    } else {
        AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
        if (!account) {
            return;
        }

        // try to match the contact info
        QContactFetchRequest *request = new QContactFetchRequest(this);
        request->setFilter(QContactPhoneNumber::match(contact->id()));

        QObject::connect(request, &QContactAbstractRequest::stateChanged, [this, request, accountId, participantIds, message](QContactAbstractRequest::State newState) {
            // only process the results after the finished state is reached
            if (newState != QContactAbstractRequest::FinishedState) {
                return;
            }

            QContact contact;
            if (request->contacts().size() > 0) {
                contact = request->contacts().at(0);

                // Notify greeter via AccountsService about this contact so it
                // can show the details if our session is locked.
                GreeterContacts::emitContact(contact);
            }
            // wait for the contact match request to finish before showing the notification
            showNotificationForMessage(message, accountId, participantIds, contact);
        });

        // FIXME: For accounts not based on phone numbers, don't try to match contacts for now
        if (account->type() == AccountEntry::PhoneAccount || account->type() == AccountEntry::MultimediaAccount) {
            request->setManager(ContactUtils::sharedManager());
            request->start();
        } else {
            // just emit the signal to pretend we did a contact search
            Q_EMIT request->stateChanged(QContactAbstractRequest::FinishedState);
        }
    }
}

void TextChannelObserver::showNotificationForMessage(const Tp::ReceivedMessage &message, const QString &accountId, const QStringList &participantIds, const QContact &contact)
{
    Tp::ContactPtr telepathyContact = message.sender();
    QString messageText = message.text();

    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);

    Tp::MessagePartList messageParts = message.parts();
    bool mms = message.header()["x-canonical-mms"].variant().toBool();
    if (mms) {
        // remove header
        messageParts.pop_front();
        Q_FOREACH(const Tp::MessagePart &part, messageParts) {
            if (part["content-type"].variant().toString().startsWith("text/plain")) {
                messageText = part["content"].variant().toString();
                break;
            }
        }
    }

    // if messageText is empty, search for attachments
    if (messageText.isEmpty()) {
        int imageCount = 0;
        int videoCount = 0;
        int contactCount = 0;
        int audioCount = 0;
        int attachmentCount = 0;
        Q_FOREACH(const Tp::MessagePart &part, messageParts) {
            QString contentType = part["content-type"].variant().toString();
            if (contentType.startsWith("image/")) {
                imageCount++;
            } else if (contentType.startsWith("video/")) {
                videoCount++;
            } else if (contentType.startsWith("audio/")) {
                audioCount++;
            } else if (contentType.startsWith("text/vcard") ||
                      contentType.startsWith("text/x-vcard")) {
                contactCount++;
            }
        }
        attachmentCount = imageCount + videoCount + contactCount;

        if (imageCount > 0 && attachmentCount == imageCount) {
            messageText = QString::fromUtf8(C::ngettext("Attachment: %1 image", "Attachments: %1 images", imageCount)).arg(imageCount);
        } else if (videoCount > 0 && attachmentCount == videoCount) {
            messageText = QString::fromUtf8(C::ngettext("Attachment: %1 video", "Attachments: %1 videos", videoCount)).arg(videoCount);
        } else if (contactCount > 0 && attachmentCount == contactCount) {
            messageText = QString::fromUtf8(C::ngettext("Attachment: %1 contact", "Attachments: %1 contacts", contactCount)).arg(contactCount);
        } else if (audioCount > 0 && attachmentCount == audioCount) {
            messageText = QString::fromUtf8(C::ngettext("Attachment: %1 audio clip", "Attachments: %1 audio clips", audioCount)).arg(audioCount);
        } else if (attachmentCount > 0) {
            messageText = QString::fromUtf8(C::ngettext("Attachment: %1 file", "Attachments: %1 files", attachmentCount)).arg(attachmentCount);
        }
    }

    // add the message to the messaging menu (use hex format to avoid invalid characters)
    QByteArray token(message.messageToken().toUtf8());

    // if the message was already read, just play the ringtone and return
    // ignore logic if we are in greeter mode
    if (!mUnreadMessages.contains(token) && !GreeterContacts::isGreeterMode()) {
        Ringtone::instance()->playIncomingMessageSound();
        return;
    }

    MessagingMenu::instance()->addMessage(telepathyContact->id(), telepathyContact->alias(), participantIds, accountId, token.toHex(), message.received(), messageText);

    QString alias;
    QString avatar;

    if (!contact.isEmpty()) {
        alias = contact.detail<QContactDisplayLabel>().label();
        avatar = contact.detail<QContactAvatar>().imageUrl().toEncoded();
    }

    if (alias.isEmpty()) {
        alias = telepathyContact->alias();
        if (alias == OFONO_UNKNOWN_NUMBER) {
            alias = C::gettext("Unknown number");
        }
    }
    if (avatar.isEmpty()) {
        avatar = QUrl(telephonyServiceDir() + "assets/avatar-default@18.png").toEncoded();
    }

    QString title;
    if (participantIds.size() > 1) {
        title = QString::fromUtf8(C::gettext("Message to group from %1")).arg(alias);
        GIcon *icon = g_themed_icon_new("contact-group");
        avatar = g_icon_to_string(icon);
        g_object_unref(icon);
    } else {
        title = alias;
    }

    AccountEntry::addAccountLabel(accountId, title);

    // show the notification
    NotifyNotification *notification = notify_notification_new(title.toStdString().c_str(),
                                                               messageText.toStdString().c_str(),
                                                               avatar.toStdString().c_str());

    // Bundle the data we need for later updating
    NotificationData *data = new NotificationData();
    data->accountId = accountId;
    data->senderId = telepathyContact->id();
    data->participantIds = participantIds;
    data->alias = alias;
    data->message = messageText;
    data->notificationList = &mNotifications;
    mNotifications.insert(notification, data);

    // add the callback action
    notify_notification_add_action (notification,
                                    "notification_action",
                                    C::gettext("View message"),
                                    notification_action,
                                    data,
                                    NULL /* will be deleted when closed */);

    notify_notification_set_hint_string(notification,
                                        "x-canonical-switch-to-application",
                                        "true");

    g_signal_connect(notification, "closed", G_CALLBACK(notification_closed), &mNotifications);

    GError *error = NULL;
    if (!notify_notification_show(notification, &error)) {
        qWarning() << "Failed to show message notification:" << error->message;
        g_error_free (error);
    }

    Ringtone::instance()->playIncomingMessageSound();
}

void TextChannelObserver::updateNotifications(const QContact &contact)
{
    QMap<NotifyNotification*, NotificationData*>::const_iterator i = mNotifications.constBegin();
    while (i != mNotifications.constEnd()) {
        NotifyNotification *notification = i.key();
        NotificationData *data = i.value();

        AccountEntry *account = TelepathyHelper::instance()->accountForId(data->accountId);
        if (!account || (account->type() != AccountEntry::PhoneAccount && account->type() != AccountEntry::MultimediaAccount)) {
            return;
        }

        // FIXME: add support for contact matching for non phone number based accounts
        Q_FOREACH(const QContactPhoneNumber phoneNumber, contact.details(QContactDetail::TypePhoneNumber)) {
            if (PhoneUtils::comparePhoneNumbers(data->senderId, phoneNumber.number()) > PhoneUtils::NO_MATCH) {
                QString displayLabel = contact.detail<QContactDisplayLabel>().label();
                QString title = QString::fromUtf8(C::gettext("Message from %1")).arg(displayLabel.isEmpty() ? data->alias : displayLabel);
                QString avatar = contact.detail<QContactAvatar>().imageUrl().toEncoded();

                if (avatar.isEmpty()) {
                    avatar = QUrl(telephonyServiceDir() + "assets/avatar-default@18.png").toEncoded();
                }

                notify_notification_update(notification,
                                           title.toStdString().c_str(),
                                           data->message.toStdString().c_str(),
                                           avatar.toStdString().c_str());

                GError *error = NULL;
                if (!notify_notification_show(notification, &error)) {
                    qWarning() << "Failed to show message notification:" << error->message;
                    g_error_free (error);
                }
            }
        }
        ++i;
    }
}

void TextChannelObserver::onTextChannelAvailable(Tp::TextChannelPtr textChannel)
{
    connect(textChannel.data(),
            SIGNAL(invalidated(Tp::DBusProxy*,const QString&, const QString&)),
            SLOT(onTextChannelInvalidated()));
    connect(textChannel.data(),
            SIGNAL(messageReceived(const Tp::ReceivedMessage&)),
            SLOT(onMessageReceived(const Tp::ReceivedMessage&)));
    connect(textChannel.data(),
            SIGNAL(pendingMessageRemoved(const Tp::ReceivedMessage&)),
            SLOT(onPendingMessageRemoved(const Tp::ReceivedMessage&)));
    connect(textChannel.data(),
            SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
            SLOT(onMessageSent(Tp::Message,Tp::MessageSendingFlags,QString)));


    if (textChannel->immutableProperties().contains(TP_QT_IFACE_CHANNEL_INTERFACE_SMS + QLatin1String(".Flash")) && 
           textChannel->immutableProperties()[TP_QT_IFACE_CHANNEL_INTERFACE_SMS + QLatin1String(".Flash")].toBool()) {
        AccountEntry *account = TelepathyHelper::instance()->accountForConnection(textChannel->connection());
        if (!account) {
            return;
        }
        // class 0 sms
        mFlashChannels.append(textChannel);
        Q_FOREACH(Tp::ReceivedMessage message, textChannel->messageQueue()) {
            showNotificationForFlashMessage(message, account->accountId());
        }
        return;
    } else {
        mChannels.append(textChannel);
    }
    // notify all the messages from the channel
    Q_FOREACH(Tp::ReceivedMessage message, textChannel->messageQueue()) {
        processMessageReceived(message, textChannel);
    }
}

void TextChannelObserver::onTextChannelInvalidated()
{
    Tp::TextChannelPtr textChannel(qobject_cast<Tp::TextChannel*>(sender()));
    mChannels.removeAll(textChannel);
    mFlashChannels.removeAll(textChannel);
}

void TextChannelObserver::processMessageReceived(const Tp::ReceivedMessage &message, const Tp::TextChannelPtr &textChannel)
{
    if (textChannel.isNull()) {
        qDebug() << "TextChannelObserver::processMessageReceived: no text channel";
        return;
    }

    AccountEntry *account = TelepathyHelper::instance()->accountForConnection(textChannel->connection());
    if (!account) {
        return;
    }

    // we do not notify messages sent by ourselves on other devices, unless we
    // are dealing with phone accounts: #1547462
    if (!account->account()->connection().isNull() && 
            message.sender()->handle().at(0) == account->account()->connection()->selfHandle() &&
            account->type() != AccountEntry::PhoneAccount) {
        return;
    }
    
    // do not place notification items for scrollback messages
    if (mFlashChannels.contains(textChannel) && !message.isScrollback() && !message.isDeliveryReport() && !message.isRescued()) {
        showNotificationForFlashMessage(message, account->accountId());
        return;
    }

    QStringList participantIds;
    Q_FOREACH(const Tp::ContactPtr &contact, textChannel->groupContacts(false)) {
        participantIds << contact->id();
    }

    if (!message.isScrollback() && !message.isDeliveryReport() && !message.isRescued()) {
        QTimer *timer = new QTimer(this);
        timer->setInterval(1500);
        timer->setSingleShot(true);
        QByteArray token(message.messageToken().toUtf8());
        mUnreadMessages.append(token);
        QObject::connect(timer, &QTimer::timeout, [=]() {
            triggerNotificationForMessage(message, account->accountId(), participantIds);
            Metrics::instance()->increment(Metrics::ReceivedMessages);
            timer->deleteLater();
        });
        timer->start();
    }
}

void TextChannelObserver::onMessageReceived(const Tp::ReceivedMessage &message)
{
    Tp::TextChannelPtr textChannel(qobject_cast<Tp::TextChannel*>(sender()));
    processMessageReceived(message, textChannel);
}

void TextChannelObserver::onPendingMessageRemoved(const Tp::ReceivedMessage &message)
{
    QByteArray token(message.messageToken().toUtf8());
    mUnreadMessages.removeAll(token);
    MessagingMenu::instance()->removeMessage(token.toHex());
}

void TextChannelObserver::onReplyReceived(const QStringList &recipients, const QString &accountId, const QString &reply)
{
    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
    if (!account) {
        return;
    }

    // FIXME - we need to find a better way to deal with dual sim in the messaging-menu
    if (!TelepathyHelper::instance()->defaultMessagingAccount()
            && TelepathyHelper::instance()->activeAccounts().size() > 1
            && account->type() != AccountEntry::GenericAccount) {
        NotifyNotification *notification = notify_notification_new(C::gettext("Please, select a SIM card:"),
                                                                   reply.toStdString().c_str(),
                                                                   "");
        NotificationData *data = new NotificationData();
        data->participantIds = recipients;
        data->accountId = accountId;
        data->text = reply;
        data->observer = this;
        mNotifications.insert(notification, data);

        Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->activeAccounts()) {
            notify_notification_add_action (notification,
                                            account->accountId().toStdString().c_str(),
                                            account->displayName().toStdString().c_str(),
                                            sim_selection_action,
                                            data,
                                            NULL);
        }
        g_signal_connect(notification, "closed", G_CALLBACK(notification_closed), &mNotifications);

        notify_notification_set_hint_string(notification,
                                            "x-canonical-snap-decisions",
                                            "true");

        GError *error = NULL;
        if (!notify_notification_show(notification, &error)) {
            qWarning() << "Failed to show message notification:" << error->message;
            g_error_free (error);
        }
        return;
    }

    if (account->type() == AccountEntry::GenericAccount) {
        sendMessage(recipients, reply, accountId);
    } else {
        // FIXME: for now we dont specify any accountId
        sendMessage(recipients, reply, "");
    }
}

void TextChannelObserver::onMessageRead(const QStringList &recipients, const QString &accountId, const QString &encodedMessageId)
{
    QString messageId(QByteArray::fromHex(encodedMessageId.toUtf8()));
    ChatManager::instance()->acknowledgeMessage(recipients, messageId, accountId);
}

void TextChannelObserver::onMessageSent(Tp::Message, Tp::MessageSendingFlags, QString)
{
    Metrics::instance()->increment(Metrics::SentMessages);
}
