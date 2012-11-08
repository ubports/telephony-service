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
#include "modelsectioncounter.h"

#include <QQmlEngine>
#include <qqml.h>
#include <TelepathyQt/Debug>
#include <TelepathyLoggerQt/Init>

#include <glib-object.h>

void Components::initializeEngine(QQmlEngine *engine, const char *uri)
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

    // check which contact engine to use
    QString contactEngine = mRootContext->contextProperty("contactEngine").toString();
    if (contactEngine.isNull()) {
        contactEngine = "folks";
    }
    mRootContext->setContextProperty("contactModel", ContactModel::instance(contactEngine));

    mCallLogModel = new CallLogModel(this);
    mRootContext->setContextProperty("callLogModel", mCallLogModel);
    mMessageLogModel = new MessageLogModel(this);
    mRootContext->setContextProperty("messageLogModel", mMessageLogModel);
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
    qmlRegisterType<ModelSectionCounter>(uri, 0, 1, "ModelSectionCounter");
}

void Components::onAccountReady()
{
    connect(TelepathyHelper::instance()->channelObserver(), SIGNAL(callEnded(const Tp::CallChannelPtr&)),
            mCallLogModel, SLOT(onCallEnded(const Tp::CallChannelPtr&)));
    connect(ChatManager::instance(), SIGNAL(messageReceived(const QString&, const QString&, const QDateTime&, const QString&)),
            mMessageLogModel, SLOT(onMessageReceived(const QString&, const QString&, const QDateTime&, const QString&)));
    connect(ChatManager::instance(), SIGNAL(messageSent(const QString&, const QString&)),
            mMessageLogModel, SLOT(onMessageSent(const QString&, const QString&)));

    mCallLogModel->populate();
    mMessageLogModel->populate();
}
