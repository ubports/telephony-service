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
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>
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
            QStringList recipients;
            recipients << notificationData->senderId << notificationData->participantIds;
            History::Thread thread = History::Manager::instance()->threadForParticipants(notificationData->accountId,
                                                                                         History::EventTypeText,
                                                                                         recipients,
                                                                                         History::MatchPhoneNumber,
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
        // TODO: open mms thread when applicable
        QStringList recipients;
        if (!notificationData->senderId.isEmpty()) {
            recipients << notificationData->senderId;
        }
        recipients << notificationData->participantIds;
        recipients.removeDuplicates();
        ApplicationUtils::openUrl(QString("message:///%1").arg(QString(QUrl::toPercentEncoding(recipients.join(";")))));
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
        if (TelepathyHelper::instance()->defaultMessagingAccount() && TelepathyHelper::instance()->activeAccounts().size() > 1) {
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
        History::Thread thread = History::Manager::instance()->threadForParticipants(account->accountId(),
                                                                                     History::EventTypeText,
                                                                                     recipients,
                                                                                     History::MatchPhoneNumber,
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
        if (account->simLocked()) {
            failureMessage = C::gettext("Unlock your sim card and try again from the messaging application.");
        } else if (TelepathyHelper::instance()->flightMode()) {
            failureMessage = C::gettext("Deactivate flight mode and try again from the messaging application.");
        } else {
            // generic error
            failureMessage = C::gettext("Try again from the messaging application.");
        }

        // notify user about the failure
        GIcon *icon = g_themed_icon_new("cancel");
        NotifyNotification *notification = notify_notification_new(C::gettext("The message could not be sent"),
                                                               failureMessage.toStdString().c_str(),
                                                               g_icon_to_string(icon));
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

    ChatManager::instance()->sendMessage(recipients, text, accountId);
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
    if (GreeterContacts::isGreeterMode()) { // we're in the greeter's session
        GreeterContacts::instance()->setContactFilter(QContactPhoneNumber::match(contact->id()));
        // in greeter mode we show the notification right away as the contact data might not be received
        showNotificationForMessage(message, accountId, participantIds);
    } else {
        // try to match the contact info
        QContactFetchRequest *request = new QContactFetchRequest(this);
        request->setFilter(QContactPhoneNumber::match(contact->id()));

        QObject::connect(request, &QContactAbstractRequest::stateChanged, [this, request, accountId, participantIds, message]() {
            // only process the results after the finished state is reached
            if (request->state() != QContactAbstractRequest::FinishedState) {
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

        request->setManager(ContactUtils::sharedManager());
        request->start();
    }

}

void TextChannelObserver::showNotificationForMessage(const Tp::ReceivedMessage &message, const QString &accountId, const QStringList &participantIds, const QContact &contact)
{
    Tp::ContactPtr telepathyContact = message.sender();
    QString messageText = message.text();

    Tp::MessagePartList messageParts = message.parts();
    bool mms = message.header()["mms"].variant().toBool();
    if (mms) {
        // remove header
        messageParts.pop_front();
        Q_FOREACH(const Tp::MessagePart &part, messageParts) {
            if (part["content-type"].variant().toString().startsWith("text/plain")) {
                messageText = part["content"].variant().toString();
                break;
            }
        }
        // WORKAROUND: powerd can't decide when to wake up the screen on incoming mms's
        // as the download of the attachments is made by another daemon, so we wake up
        // the screen here.
        if (!CallManager::instance()->hasCalls()) {
            QDBusInterface unityIface("com.canonical.Unity.Screen",
                                      "/com/canonical/Unity/Screen",
                                      "com.canonical.Unity.Screen",
                                      QDBusConnection::systemBus());
            QList<QVariant> args;
            args.append("on");
            args.append(0);
            unityIface.callWithArgumentList(QDBus::NoBlock, "setScreenPowerMode", args);
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

    MessagingMenu::instance()->addMessage(telepathyContact->id(), participantIds, accountId, token.toHex(), message.received(), messageText);

    QString alias;
    QString avatar;

    if (!contact.isEmpty()) {
        alias = contact.detail<QContactDisplayLabel>().label();
        avatar = contact.detail<QContactAvatar>().imageUrl().toEncoded();
    }

    if (alias.isEmpty()) {
        alias = telepathyContact->alias();
    }
    if (avatar.isEmpty()) {
        avatar = QUrl(telephonyServiceDir() + "assets/avatar-default@18.png").toEncoded();
    }

    QString title;
    if (participantIds.isEmpty()) {
        title = QString::fromUtf8(C::gettext("Message from %1")).arg(alias);
    } else {
        title = QString::fromUtf8(C::gettext("Message to group from %1")).arg(alias);
    }
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
        Q_FOREACH(const QContactPhoneNumber phoneNumber, contact.details(QContactDetail::TypePhoneNumber)) {
            if (PhoneUtils::comparePhoneNumbers(data->senderId, phoneNumber.number())) {
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
    // FIXME - we need to find a better way to deal with dual sim in the messaging-menu
    if (!TelepathyHelper::instance()->defaultMessagingAccount() && TelepathyHelper::instance()->activeAccounts().size() > 1) {
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

    // FIXME: for now we dont specify any accountId
    sendMessage(recipients, reply, "");
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
