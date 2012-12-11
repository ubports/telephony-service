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

#ifndef TEXTCHANNELOBSERVER_H
#define TEXTCHANNELOBSERVER_H

#include <QObject>
#include <TelepathyQt/AbstractClientObserver>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>

class TextChannelObserver : public QObject, public Tp::AbstractClientObserver
{
    Q_OBJECT
public:
    explicit TextChannelObserver(QObject *parent = 0);

    Tp::ChannelClassSpecList channelFilters() const;

    void observeChannels(const Tp::MethodInvocationContextPtr<> &context,
                         const Tp::AccountPtr &account,
                         const Tp::ConnectionPtr &connection,
                         const QList<Tp::ChannelPtr> &channels,
                         const Tp::ChannelDispatchOperationPtr &dispatchOperation,
                         const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                         const Tp::AbstractClientObserver::ObserverInfo &observerInfo);

protected:
    void showNotificationForMessage(const Tp::ReceivedMessage &message);
    Tp::TextChannelPtr channelFromPath(const QString &path);

protected Q_SLOTS:
    void onTextChannelReady(Tp::PendingOperation *op);
    void onTextChannelInvalidated();
    void onMessageReceived(const Tp::ReceivedMessage &message);
    void onPendingMessageRemoved(const Tp::ReceivedMessage &message);

private:
    QMap<Tp::Channel*, Tp::MethodInvocationContextPtr<> > mContexts;
    QMap<Tp::PendingReady*, Tp::ChannelPtr> mReadyMap;
    QList<Tp::TextChannelPtr> mChannels;
};

#endif // TEXTCHANNELOBSERVER_H
