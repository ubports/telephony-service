/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#ifndef CHANNELHANDLER_H
#define CHANNELHANDLER_H

#include <TelepathyQt/AbstractClientHandler>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/ChannelRequest>

class ChannelHandler : public QObject, public Tp::AbstractClientHandler
{
    Q_OBJECT

public:
    ChannelHandler(QObject *parent = 0);
    ~ChannelHandler() { }
    bool bypassApproval() const;
    void handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                        const Tp::AccountPtr &account,
                        const Tp::ConnectionPtr &connection,
                        const QList<Tp::ChannelPtr> &channels,
                        const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                        const QDateTime &userActionTime,
                        const Tp::AbstractClientHandler::HandlerInfo &handlerInfo);
    Tp::ChannelClassSpecList channelFilters();

Q_SIGNALS:
    void textChannelAvailable(Tp::TextChannelPtr textChannel);
    void callChannelAvailable(Tp::CallChannelPtr callChannel);

private Q_SLOTS:
    void onTextChannelReady(Tp::PendingOperation *op);
    void onCallChannelReady(Tp::PendingOperation *op);
    void onChannelRequested(Tp::ChannelRequestPtr channelRequest);

private:
    QMap<Tp::PendingReady*, Tp::ChannelPtr> mReadyRequests;
    QList<Tp::ChannelRequestPtr> mChannelRequests;
};

#endif
