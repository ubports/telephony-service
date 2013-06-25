/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libnotify/notify.h>
#include "textchannelobserver.h"
#include "messagingmenu.h"
#include "contactmodel.h"
#include "contactentry.h"
#include "config.h"
#include "ringtone.h"
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>
#include <QImage>

namespace C {
#include <libintl.h>
}

TextChannelObserver::TextChannelObserver(QObject *parent) :
    QObject(parent)
{
}

void TextChannelObserver::showNotificationForMessage(const Tp::ReceivedMessage &message)
{
    // do not place notification items for scrollback messages
    if (message.isScrollback() || message.isDeliveryReport() || message.isRescued()) {
        return;
    }

    Tp::ContactPtr contact = message.sender();
    QString title = QString::fromUtf8(C::gettext("SMS from %1")).arg(contact->alias());
    QString icon = contact->avatarData().fileName;
    ContactEntry *entry = ContactModel::instance()->contactFromPhoneNumber(contact->id());
    if (entry) {
        title = QString("SMS from %1").arg(entry->displayLabel());
        icon = entry->avatar().toLocalFile();
    }

    if (icon.isEmpty()) {
        icon = phoneAppDirectory() + "/assets/avatar-default@18.png";
    }

    // show the notification
    NotifyNotification *notification = notify_notification_new(title.toStdString().c_str(),
                                                               message.text().toStdString().c_str(),
                                                               icon.toStdString().c_str());
    GError *error = NULL;
    if (!notify_notification_show(notification, &error)) {
        qWarning() << "Failed to show message notification:" << error->message;
        g_error_free (error);
    }

    g_object_unref(G_OBJECT(notification));

    // and add the message to the messaging menu (use hex format to avoid invalid characters)  
    QByteArray token(message.messageToken().toUtf8()); 
    MessagingMenu::instance()->addMessage(contact->id(), token.toHex(), message.received(), message.text());
    Ringtone::instance()->playIncomingMessageSound();
}

Tp::TextChannelPtr TextChannelObserver::channelFromPath(const QString &path)
{
    Q_FOREACH(Tp::TextChannelPtr channel, mChannels) {
        if (channel->objectPath() == path) {
            return channel;
        }
    }

    return Tp::TextChannelPtr(0);
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

    mChannels.append(textChannel);

    // notify all the messages from the channel
    Q_FOREACH(Tp::ReceivedMessage message, textChannel->messageQueue()) {
        showNotificationForMessage(message);
    }
}

void TextChannelObserver::onTextChannelInvalidated()
{
    Tp::TextChannelPtr textChannel(qobject_cast<Tp::TextChannel*>(sender()));
    mChannels.removeAll(textChannel);
}

void TextChannelObserver::onMessageReceived(const Tp::ReceivedMessage &message)
{
    showNotificationForMessage(message);
}

void TextChannelObserver::onPendingMessageRemoved(const Tp::ReceivedMessage &message)
{
    MessagingMenu::instance()->removeMessage(message.messageToken());
}
