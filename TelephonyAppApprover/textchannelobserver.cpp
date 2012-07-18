/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libnotify/notify.h>
#include "textchannelobserver.h"
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ChannelClassSpecList>
#include <TelepathyQt/MethodInvocationContext>
#include <TelepathyQt/ReceivedMessage>
#include <qindicateserver.h>
#include <qindicateindicator.h>
#include <QImage>

TextChannelObserver::TextChannelObserver(QObject *parent) :
    QObject(parent), Tp::AbstractClientObserver(channelFilters(), true)
{
    mIndicateServer = QIndicate::Server::defaultInstance();
    mIndicateServer->setType("message");
    mIndicateServer->setDesktopFile("/usr/share/applications/telephony-app-sms.desktop");
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

void TextChannelObserver::updateIndicatorsFromChannel(const Tp::TextChannelPtr &textChannel)
{
    if (textChannel->messageQueue().isEmpty()) {
        // no more pending messages, remove the indicator
        removeIndicatorsFromChannel(textChannel);
    } else {
        QIndicate::Indicator *indicator;
        if (mIndicators.contains(textChannel->objectPath())) {
            indicator = mIndicators[textChannel->objectPath()];
        } else {
            indicator = new QIndicate::Indicator(this);
            mIndicators[textChannel->objectPath()] = indicator;
            indicator->setProperty("channelPath", textChannel->objectPath());

            connect(indicator,
                    SIGNAL(display(QIndicate::Indicator*)),
                    SLOT(onIndicatorDisplay(QIndicate::Indicator*)));
        }

        indicator->setNameProperty(textChannel->targetContact()->alias());
        indicator->setIconProperty(QImage(textChannel->targetContact()->avatarData().fileName));
        indicator->setCountProperty(textChannel->messageQueue().count());
        indicator->setTimeProperty(textChannel->messageQueue().last().received());
        indicator->show();
    }
}

void TextChannelObserver::removeIndicatorsFromChannel(const Tp::TextChannelPtr &textChannel)
{
    if (!mIndicators.contains(textChannel->objectPath())) {
        return;
    }

    delete mIndicators.take(textChannel->objectPath());
}

void TextChannelObserver::showNotificationForMessage(const Tp::ReceivedMessage &message)
{
    // do not place notification items for scrollback messages
    if (message.isScrollback() || message.isDeliveryReport() || message.isRescued()) {
        return;
    }

    Tp::ContactPtr contact = message.sender();
    QString title = QString("New SMS message from %1").arg(contact->alias());
    QString icon = contact->avatarData().fileName;
    if (icon.isEmpty()) {
        icon = "mail-unread";
    }

    NotifyNotification *notification = notify_notification_new(title.toStdString().c_str(),
                                                               message.text().toStdString().c_str(),
                                                               icon.toStdString().c_str());
    GError *error = NULL;
    if (!notify_notification_show(notification, &error)) {
        qWarning() << "Failed to show snap decision:" << error->message;
        g_error_free (error);
    }

    g_object_unref(G_OBJECT(notification));
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
    updateIndicatorsFromChannel(textChannel);

    // notify the latest message from the channel, if any
    if (!textChannel->messageQueue().isEmpty()) {
        showNotificationForMessage(textChannel->messageQueue().last());
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
    removeIndicatorsFromChannel(textChannel);
    mChannels.removeAll(textChannel);
}

void TextChannelObserver::onMessageReceived(const Tp::ReceivedMessage &message)
{
    Tp::TextChannelPtr textChannel(qobject_cast<Tp::TextChannel*>(sender()));
    updateIndicatorsFromChannel(textChannel);

    showNotificationForMessage(message);
}

void TextChannelObserver::onPendingMessageRemoved(const Tp::ReceivedMessage &message)
{
    Tp::TextChannelPtr textChannel(qobject_cast<Tp::TextChannel*>(sender()));
    updateIndicatorsFromChannel(textChannel);
}

void TextChannelObserver::onIndicatorDisplay(QIndicate::Indicator *indicator)
{
    Tp::TextChannelPtr channel = channelFromPath(indicator->property("channelPath").toString());
    if (channel.isNull()) {
        qWarning() << "Unable to find the text channel corresponding to the indicator" << indicator;
    }

    QString id = channel->targetContact()->id();
    QDBusInterface telephonyApp("com.canonical.TelephonyApp",
                                "/com/canonical/TelephonyApp",
                                "com.canonical.TelephonyApp");
    telephonyApp.call("ShowMessages", id);
}
