/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
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
#include "config.h"
#include "ringtone.h"
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ChannelClassSpecList>
#include <TelepathyQt/MethodInvocationContext>
#include <TelepathyQt/ReceivedMessage>
#include <QImage>

TextChannelObserver::TextChannelObserver(QObject *parent) :
    QObject(parent), Tp::AbstractClientObserver(channelFilters(), true)
{    
}

Tp::ChannelClassSpecList TextChannelObserver::channelFilters() const
{
    Tp::ChannelClassSpecList specList;
    specList << Tp::ChannelClassSpec::textChat();

    return specList;
}

void TextChannelObserver::observeChannels(const Tp::MethodInvocationContextPtr<> &context,
                                      const Tp::AccountPtr &account,
                                      const Tp::ConnectionPtr &connection,
                                      const QList<Tp::ChannelPtr> &channels,
                                      const Tp::ChannelDispatchOperationPtr &dispatchOperation,
                                      const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                                      const Tp::AbstractClientObserver::ObserverInfo &observerInfo)
{
    Q_UNUSED(account)
    Q_UNUSED(connection)
    Q_UNUSED(dispatchOperation)
    Q_UNUSED(requestsSatisfied)
    Q_UNUSED(observerInfo)

    Q_FOREACH (Tp::ChannelPtr channel, channels) {
        Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(channel);
        if (!textChannel) {
            qWarning() << "Observed channel is not a text channel:" << channel;
            continue;
        }

        Tp::PendingReady *ready = textChannel->becomeReady(Tp::Features()
                                                           << Tp::TextChannel::FeatureCore
                                                           << Tp::TextChannel::FeatureMessageQueue
                                                           << Tp::TextChannel::FeatureChatState);
        connect(ready,
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onTextChannelReady(Tp::PendingOperation*)));
        mReadyMap[ready] = textChannel;
        mContexts[textChannel.data()] = context;
    }
}

void TextChannelObserver::showNotificationForMessage(const Tp::ReceivedMessage &message)
{
    // do not place notification items for scrollback messages
    if (message.isScrollback() || message.isDeliveryReport() || message.isRescued()) {
        return;
    }

    Tp::ContactPtr contact = message.sender();
    QString title = QString("SMS from %1").arg(contact->alias());
    QString icon = contact->avatarData().fileName;
    if (icon.isEmpty()) {
        icon = telephonyAppDirectory() + "/assets/avatar-default@18.png";
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

    // and add the message to the messaging menu
    MessagingMenu::instance()->addMessage(contact->id(), message.messageToken(), message.received(), message.text());
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

void TextChannelObserver::onTextChannelReady(Tp::PendingOperation *op)
{
    Tp::PendingReady *ready = qobject_cast<Tp::PendingReady*>(op);
    if (!ready) {
        qCritical() << "Pending operation is not a pending ready:" << op;
        return;
    }

    if (!mReadyMap.contains(ready)) {
        qWarning() << "Pending ready finished but not on the map:" << ready;
        return;
    }

    Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(mReadyMap[ready]);
    mReadyMap.remove(ready);

    if (!textChannel) {
        qWarning() << "Ready channel is not a call channel:" << mReadyMap[ready];
        return;
    }

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

    if (!mContexts.contains(textChannel.data())) {
        qWarning() << "Context for channel not available:" << textChannel;
        return;
    }

    Tp::MethodInvocationContextPtr<> context = mContexts[textChannel.data()];
    mContexts.remove(textChannel.data());

    // check if this is the last channel from the context
    Q_FOREACH(Tp::MethodInvocationContextPtr<> otherContext, mContexts.values()) {
        // if we find the context, just return from the function. We need to wait
        // for the other channels to become ready before setting the context finished
        if (otherContext == context) {
            return;
        }
    }

    context->setFinished();
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
