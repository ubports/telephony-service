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
#include "calllogmodel.h"
#include "calllogproxymodel.h"
#include "conversationlogmodel.h"
#include "contactmanager.h"
#include "messagelogmodel.h"
#include "messagesproxymodel.h"

#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/qdeclarative.h>
#include <TelepathyQt/Debug>
#include <TelepathyLoggerQt4/Init>

#include <glib-object.h>

void Components::initializeEngine(QDeclarativeEngine *engine, const char *uri)
{
    Q_ASSERT(engine);

    Q_UNUSED(uri);

    // needed for tp-logger
    g_type_init();

    // Initialize telepathy types
    Tp::registerTypes();
    //Tp::enableDebug(true);
    Tp::enableWarnings(true);
    Tpl::init();

    mRootContext = engine->rootContext();
    Q_ASSERT(mRootContext);

    connect(TelepathyHelper::instance(),
            SIGNAL(accountReady()),
            SLOT(onAccountReady()));

    mRootContext->setContextProperty("contactManager", ContactManager::instance());
    mRootContext->setContextProperty("telepathyManager", TelepathyHelper::instance());
    mRootContext->setContextProperty("chatManager", TelepathyHelper::instance()->chatManager());
    mRootContext->setContextProperty("callManager", TelepathyHelper::instance()->callManager());

    connect(TelepathyHelper::instance(),
            SIGNAL(channelHandlerCreated(ChannelHandler*)),
            SLOT(onChannelHandlerCreated(ChannelHandler*)));
}

void Components::registerTypes(const char *uri)
{
    // @uri TelephonyApp
    qmlRegisterUncreatableType<TelepathyHelper>(uri, 0, 1, "TelepathyHelper", "This is a singleton helper class");
    qmlRegisterType<CallLogProxyModel>(uri, 0, 1, "CallLogProxyModel");
    qmlRegisterType<MessagesProxyModel>(uri, 0, 1, "MessagesProxyModel");
}

void Components::onChannelHandlerCreated(ChannelHandler *ch)
{
    // register the context property
    mRootContext->setContextProperty("channelHandler", ch);
    mRootContext->setContextProperty("telepathyManager", ch);
}

void Components::onAccountReady()
{
    // create the log models just when the telepathy helper signals the account is ready
    mCallLogModel = new CallLogModel(this);
    mRootContext->setContextProperty("callLogModel", mCallLogModel);

    mConversationLogModel = new ConversationLogModel(this);
    mRootContext->setContextProperty("conversationLogModel", mConversationLogModel);
    connect(TelepathyHelper::instance()->chatManager(), SIGNAL(messageReceived(const QString&, const QString&)),
            mConversationLogModel, SLOT(onMessageReceived(const QString&, const QString&)));
    connect(TelepathyHelper::instance()->chatManager(), SIGNAL(messageSent(const QString&, const QString&)),
            mConversationLogModel, SLOT(onMessageReceived(const QString&, const QString&)));

    mMessageLogModel = new MessageLogModel(this);
    mRootContext->setContextProperty("messageLogModel", mMessageLogModel);
    connect(TelepathyHelper::instance()->chatManager(), SIGNAL(messageReceived(const QString&, const QString&)),
            mMessageLogModel, SLOT(onMessageReceived(const QString&, const QString&)));
    connect(TelepathyHelper::instance()->chatManager(), SIGNAL(messageSent(const QString&, const QString&)),
            mMessageLogModel, SLOT(onMessageReceived(const QString&, const QString&)));
}

Q_EXPORT_PLUGIN2(components, Components)
