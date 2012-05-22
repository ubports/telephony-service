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

#include "channelhandler.h"

#include <TelepathyQt/CallChannel>

ChannelHandler::ChannelHandler(const Tp::ChannelClassSpecList &channelFilter)
    : Tp::AbstractClientHandler(channelFilter)
{
}

bool ChannelHandler::bypassApproval() const
{
    return false;
}

void ChannelHandler::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                               const Tp::AccountPtr &account,
                               const Tp::ConnectionPtr &connection,
                               const QList<Tp::ChannelPtr> &channels,
                               const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                               const QDateTime &userActionTime,
                               const Tp::AbstractClientHandler::HandlerInfo &handlerInfo)
{
    foreach(const Tp::ChannelPtr &channel, channels) {
        QVariantMap properties = channel->immutableProperties();
        if (properties[QString(TP_QT_IFACE_CHANNEL) + ".ChannelType"] ==
                TP_QT_IFACE_CHANNEL_TYPE_CALL) {
            if (channel->isRequested()) {
            } else {
            }
        }
    }
    context->setFinished();
}

