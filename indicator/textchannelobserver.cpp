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

#include "applicationutils.h"
#include "greetercontacts.h"
#include "textchannelobserver.h"
#include "messagingmenu.h"
#include "metrics.h"
#include "chatmanager.h"
#include "config.h"
#include "contactutils.h"
#include "ringtone.h"
#include "telepathyhelper.h"
#include "phoneutils.h"
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>
#include <QContactAvatar>
#include <QContactFetchRequest>
#include <QContactFilter>
#include <QContactPhoneNumber>
#include <QImage>
#include <History/TextEvent>
#include <History/Manager>

namespace C {
#include <libintl.h>
}

QTCONTACTS_USE_NAMESPACE

// notification handling

class NotificationData {
public:
    QString phoneNumber;
    QDateTime timestamp;
    QString text;
    QString eventId;
    QString alias;
    QString message;
    QMap<NotifyNotification*, NotificationData*> *notificationList;
};

void notification_closed(NotifyNotification *notification, QMap<NotifyNotification*, NotificationData*> *map);

void flash_notification_action(NotifyNotification* notification, char *action, gpointer data)
{
    GError *error = NULL;
    if (action == QLatin1String("notification_save_action")) {
        NotificationData *notificationData = (NotificationData*) data;
        if (notificationData != NULL) {
            // FIXME: handle multiple accounts
            History::Thread thread = History::Manager::instance()->threadForParticipants(TelepathyHelper::instance()->accountIds()[0],
                                                                                         History::EventTypeText,
                                                                                         QStringList() << notificationData->phoneNumber,
                                                                                         History::MatchPhoneNumber,
                                                                                         true);
            History::TextEvent textEvent(TelepathyHelper::instance()->accountIds()[0], 
                                         thread.threadId(), 
                                         notificationData->eventId, 
                                         notificationData->phoneNumber,
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
        ApplicationUtils::openUrl(QString("message:///%1").arg(QString(QUrl::toPercentEncoding(notificationData->phoneNumber))));
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
            SIGNAL(replyReceived(QString,QString)),
            SLOT(onReplyReceived(QString,QString)));
    connect(MessagingMenu::instance(),
            SIGNAL(messageRead(QString,QString)),
            SLOT(onMessageRead(QString,QString)));

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

void TextChannelObserver::showNotificationForFlashMessage(const Tp::ReceivedMessage &message)
{
    Tp::ContactPtr contact = message.sender();
    QByteArray token(message.messageToken().toUtf8());
    MessagingMenu::instance()->addFlashMessage(contact->id(), token.toHex(), message.received(), message.text());


    // show the notification
    NotifyNotification *notification = notify_notification_new("",
                                                               message.text().toStdString().c_str(),
                                                               "");
    NotificationData *data = new NotificationData();
    data->phoneNumber = contact->id();
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

void TextChannelObserver::showNotificationForMessage(const Tp::ReceivedMessage &message)
{
    Tp::ContactPtr contact = message.sender();
    QString messageText = message.text();

    Tp::MessagePartList messageParts = message.parts();
    // check if this is an mms
    if (messageParts.size() > 2) {
        // remove header
        messageParts.pop_front();
        Q_FOREACH(const Tp::MessagePart &part, messageParts) {
            if (part["content-type"].variant().toString().startsWith("text/plain")) {
                messageText = part["content"].variant().toString();
                break;
            }
        }
    }

    // add the message to the messaging menu (use hex format to avoid invalid characters)
    QByteArray token(message.messageToken().toUtf8());
    MessagingMenu::instance()->addMessage(contact->id(), token.toHex(), message.received(), messageText);

    QString title = QString::fromUtf8(C::gettext("Message from %1")).arg(contact->alias());
    QString avatar = QUrl(telephonyServiceDir() + "assets/avatar-default@18.png").toEncoded();

    qDebug() << title << avatar;
    // show the notification
    NotifyNotification *notification = notify_notification_new(title.toStdString().c_str(),
                                                               messageText.toStdString().c_str(),
                                                               avatar.toStdString().c_str());

    // Bundle the data we need for later updating
    NotificationData *data = new NotificationData();
    data->phoneNumber = contact->id();
    data->alias = contact->alias();
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

    if (GreeterContacts::isGreeterMode()) { // we're in the greeter's session
        GreeterContacts::instance()->setContactFilter(QContactPhoneNumber::match(contact->id()));
    } else {
        // try to match the contact info
        QContactFetchRequest *request = new QContactFetchRequest(this);
        request->setFilter(QContactPhoneNumber::match(contact->id()));

        QObject::connect(request, &QContactAbstractRequest::stateChanged, [this, request]() {
            // only process the results after the finished state is reached
            if (request->state() != QContactAbstractRequest::FinishedState ||
                request->contacts().size() == 0) {
                return;
            }

            QContact contact = request->contacts().at(0);
            updateNotifications(contact);

            // Notify greeter via AccountsService about this contact so it
            // can show the details if our session is locked.
            GreeterContacts::emitContact(contact);
        });

        request->setManager(ContactUtils::sharedManager());
        request->start();
    }

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
        Q_FOREACH(const QContactPhoneNumber phoneNumber, contact.details(QContactDetail::TypePhoneNumber)) {
            if (PhoneUtils::comparePhoneNumbers(data->phoneNumber, phoneNumber.number())) {
                QString displayLabel = ContactUtils::formatContactName(contact);
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

        // class 0 sms
        mFlashChannels.append(textChannel);
        Q_FOREACH(Tp::ReceivedMessage message, textChannel->messageQueue()) {
            showNotificationForFlashMessage(message);
        }
        return;
    } else {
        mChannels.append(textChannel);
    }
    // notify all the messages from the channel
    Q_FOREACH(Tp::ReceivedMessage message, textChannel->messageQueue()) {
        onMessageReceived(message);
    }
}

void TextChannelObserver::onTextChannelInvalidated()
{
    Tp::TextChannelPtr textChannel(qobject_cast<Tp::TextChannel*>(sender()));
    mChannels.removeAll(textChannel);
    mFlashChannels.removeAll(textChannel);
}

void TextChannelObserver::onMessageReceived(const Tp::ReceivedMessage &message)
{
    Tp::TextChannelPtr textChannel(qobject_cast<Tp::TextChannel*>(sender()));
    // do not place notification items for scrollback messages
    if (mFlashChannels.contains(textChannel) && !message.isScrollback() && !message.isDeliveryReport() && !message.isRescued()) {
        showNotificationForFlashMessage(message);
        return;
    }

    if (!message.isScrollback() && !message.isDeliveryReport() && !message.isRescued()) {
        showNotificationForMessage(message);
        Metrics::instance()->increment(Metrics::ReceivedMessages);
    }
}

void TextChannelObserver::onPendingMessageRemoved(const Tp::ReceivedMessage &message)
{
    QByteArray token(message.messageToken().toUtf8());
    MessagingMenu::instance()->removeMessage(token.toHex());
}

void TextChannelObserver::onReplyReceived(const QString &phoneNumber, const QString &reply)
{
    ChatManager::instance()->sendMessage(QStringList() << phoneNumber, reply);
}

void TextChannelObserver::onMessageRead(const QString &phoneNumber, const QString &encodedMessageId)
{
    QString messageId(QByteArray::fromHex(encodedMessageId.toUtf8()));
    ChatManager::instance()->acknowledgeMessage(phoneNumber, messageId);
}

void TextChannelObserver::onMessageSent(Tp::Message, Tp::MessageSendingFlags, QString)
{
    Metrics::instance()->increment(Metrics::SentMessages);
}
