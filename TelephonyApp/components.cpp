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
#include "channelobserver.h"
#include "chatmanager.h"
#include "calllogmodel.h"
#include "calllogproxymodel.h"
#include "contactmodel.h"
#include "contactentry.h"
#include "contactproxymodel.h"
#include "contactdetail.h"
#include "contactaddress.h"
#include "contactemailaddress.h"
#include "contactname.h"
#include "contactonlineaccount.h"
#include "contactphonenumber.h"
#include "messagelogmodel.h"
#include "messagesproxymodel.h"
#include "buttonmaskeffect.h"
#include "modelsectioncounter.h"

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

    mRootContext->setContextProperty("telepathyHelper", TelepathyHelper::instance());
    mRootContext->setContextProperty("chatManager", ChatManager::instance());
    mRootContext->setContextProperty("callManager", TelepathyHelper::instance()->callManager());
    mRootContext->setContextProperty("contactModel", ContactModel::instance());

    connect(TelepathyHelper::instance(),
            SIGNAL(channelHandlerCreated(ChannelHandler*)),
            SLOT(onChannelHandlerCreated(ChannelHandler*)));
    connect(TelepathyHelper::instance(),
            SIGNAL(channelObserverCreated(ChannelObserver*)),
            SLOT(onChannelObserverCreated(ChannelObserver*)));
}

void Components::registerTypes(const char *uri)
{
    // @uri TelephonyApp
    qmlRegisterUncreatableType<TelepathyHelper>(uri, 0, 1, "TelepathyHelper", "This is a singleton helper class");
    qmlRegisterType<CallLogProxyModel>(uri, 0, 1, "CallLogProxyModel");
    qmlRegisterType<MessagesProxyModel>(uri, 0, 1, "MessagesProxyModel");
    qmlRegisterType<ContactEntry>(uri, 0, 1, "ContactEntry");
    qmlRegisterType<ContactProxyModel>(uri, 0, 1, "ContactProxyModel");
    qmlRegisterType<ContactDetail>(uri, 0, 1, "ContactDetail");
    qmlRegisterType<ContactAddress>(uri, 0, 1, "ContactAddress");
    qmlRegisterType<ContactEmailAddress>(uri, 0, 1, "ContactEmailAddress");
    qmlRegisterType<ContactName>(uri, 0, 1, "ContactName");
    qmlRegisterType<ContactOnlineAccount>(uri, 0, 1, "ContactOnlineAccount");
    qmlRegisterType<ContactPhoneNumber>(uri, 0, 1, "ContactPhoneNumber");
    qmlRegisterType<ButtonMaskEffect>(uri, 0, 1, "ButtonMaskEffect");
    qmlRegisterType<ModelSectionCounter>(uri, 0, 1, "ModelSectionCounter");
}

void Components::onChannelHandlerCreated(ChannelHandler *handler)
{
    // register the context property
    mRootContext->setContextProperty("channelHandler", handler);
}

void Components::onChannelObserverCreated(ChannelObserver *observer)
{
    // register the context property
    mRootContext->setContextProperty("channelObserver", observer);
}

void Components::onAccountReady()
{
    // create the log models just when the telepathy helper signals the account is ready
    mCallLogModel = new CallLogModel(this);
    mRootContext->setContextProperty("callLogModel", mCallLogModel);
    connect(TelepathyHelper::instance()->channelObserver(), SIGNAL(callEnded(const Tp::CallChannelPtr&)),
            mCallLogModel, SLOT(onCallEnded(const Tp::CallChannelPtr&)));

    mMessageLogModel = new MessageLogModel(this);
    mRootContext->setContextProperty("messageLogModel", mMessageLogModel);
    connect(ChatManager::instance(), SIGNAL(messageReceived(const QString&, const QString&, const QDateTime&)),
            mMessageLogModel, SLOT(onMessageReceived(const QString&, const QString&, const QDateTime&)));
    connect(ChatManager::instance(), SIGNAL(messageSent(const QString&, const QString&)),
            mMessageLogModel, SLOT(onMessageSent(const QString&, const QString&)));
}

Q_EXPORT_PLUGIN2(components, Components)
