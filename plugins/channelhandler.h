/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include <TelepathyQt/AbstractClientHandler>

class ChannelHandler : public Tp::AbstractClientHandler
{
public:
    ChannelHandler(const Tp::ChannelClassSpecList &channelFilter);
    ~ChannelHandler() { }
    bool bypassApproval() const;
    void handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                        const Tp::AccountPtr &account,
                        const Tp::ConnectionPtr &connection,
                        const QList<Tp::ChannelPtr> &channels,
                        const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                        const QDateTime &userActionTime,
                        const Tp::AbstractClientHandler::HandlerInfo &handlerInfo);
};

