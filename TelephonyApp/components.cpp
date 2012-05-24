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

#include "components.h"
#include "telepathyhelper.h"
#include "channelhandler.h"

#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/qdeclarative.h>
#include <TelepathyQt/Debug>

void Components::initializeEngine(QDeclarativeEngine *engine, const char *uri)
{
    Q_ASSERT(engine);

    Q_UNUSED(uri);

    // Initialize telepathy types
    Tp::registerTypes();
    //Tp::enableDebug(true);
    Tp::enableWarnings(true);

    mRootContext = engine->rootContext();
    Q_ASSERT(mRootContext);

    mRootContext->setContextProperty("telepathyManager", TelepathyHelper::instance());
    mRootContext->setContextProperty("chatManager", TelepathyHelper::instance()->chatManager());

    connect(TelepathyHelper::instance(),
            SIGNAL(channelHandlerCreated(ChannelHandler*)),
            SLOT(onChannelHandlerCreated(ChannelHandler*)));
}

void Components::registerTypes(const char *uri)
{
    // @uri TelephonyApp
    qmlRegisterUncreatableType<TelepathyHelper>(uri, 0, 1, "TelepathyHelper", "This is a singleton helper class");
}

void Components::onChannelHandlerCreated(ChannelHandler *ch)
{
    // register the context property
    mRootContext->setContextProperty("channelHandler", ch);
    mRootContext->setContextProperty("telepathyManager", ch);
}

Q_EXPORT_PLUGIN2(components, Components)
