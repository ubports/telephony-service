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

#ifndef CHANNELOBSERVER_H
#define CHANNELOBSERVER_H

#include <QObject>
#include <TelepathyQt/AbstractClientObserver>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/Channel>

class ChannelObserver : public QObject, public Tp::AbstractClientObserver
{
    Q_OBJECT
public:
    explicit ChannelObserver(QObject *parent = 0);

    Tp::ChannelClassSpecList channelFilters() const;

    void observeChannels(const Tp::MethodInvocationContextPtr<> &context,
                         const Tp::AccountPtr &account,
                         const Tp::ConnectionPtr &connection,
                         const QList<Tp::ChannelPtr> &channels,
                         const Tp::ChannelDispatchOperationPtr &dispatchOperation,
                         const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                         const Tp::AbstractClientObserver::ObserverInfo &observerInfo);
    
Q_SIGNALS:
    void callEnded(const Tp::CallChannelPtr &channel);
    
protected Q_SLOTS:
    void onCallChannelReady(Tp::PendingOperation *op);
    void onCallChannelInvalidated();
    void onCallStateChanged(Tp::CallState state);

private:
    QMap<Tp::Channel*, Tp::MethodInvocationContextPtr<> > mContexts;
    QMap<Tp::PendingReady*, Tp::ChannelPtr> mReadyMap;
    QList<Tp::ChannelPtr> mChannels;
    
};

#endif // CHANNELOBSERVER_H
