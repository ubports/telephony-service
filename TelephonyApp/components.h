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

#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "channelhandler.h"

#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeExtensionPlugin>

class ChannelHandler;
class CallLogModel;
class ConversationLogModel;

namespace QtMobility {
    class QContactManager;
}

class Components : public QDeclarativeExtensionPlugin
{
    Q_OBJECT

public:
    void initializeEngine(QDeclarativeEngine *engine, const char *uri);
    void registerTypes(const char *uri);

private Q_SLOTS:
    void onChannelHandlerCreated(ChannelHandler *ch);
    void onAccountReady();

private:
    QDeclarativeContext *mRootContext;
    CallLogModel *mCallLogModel;
    ConversationLogModel *mConversationLogModel;
    QtMobility::QContactManager *mContactManager;
};

#endif // COMPONENTS_H
