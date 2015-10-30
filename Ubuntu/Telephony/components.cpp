/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "components.h"
#include "telepathyhelper.h"
#include "callentry.h"
#include "callmanager.h"
#include "callnotification.h"
#include "ussdmanager.h"
#include "channelobserver.h"
#include "chatmanager.h"
#include "contactwatcher.h"
#include "greetercontacts.h"
#include "phoneutils.h"
#include "protocolmanager.h"
#include "accountentry.h"
#include "audiooutput.h"

#include <QQmlEngine>
#include <qqml.h>
#include <TelepathyQt/Debug>

void Components::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_ASSERT(engine);

    Q_UNUSED(uri);

    // Initialize telepathy types
    Tp::registerTypes();
    //Tp::enableDebug(true);
    Tp::enableWarnings(true);

    mRootContext = engine->rootContext();
    Q_ASSERT(mRootContext);

    mRootContext->setContextProperty("telepathyHelper", TelepathyHelper::instance());
    mRootContext->setContextProperty("chatManager", ChatManager::instance());
    mRootContext->setContextProperty("callManager", CallManager::instance());
    mRootContext->setContextProperty("greeter", GreeterContacts::instance());
    mRootContext->setContextProperty("callNotification", CallNotification::instance());
    mRootContext->setContextProperty("protocolManager", ProtocolManager::instance());

}

void Components::registerTypes(const char *uri)
{
    // @uri Telephony
    qmlRegisterUncreatableType<TelepathyHelper>(uri, 0, 1, "TelepathyHelper", "This is a singleton helper class");
    qmlRegisterUncreatableType<CallEntry>(uri, 0, 1, "CallEntry", "Objects of this type are created in CallManager and made available to QML for usage");
    qmlRegisterUncreatableType<AudioOutput>(uri, 0, 1, "AudioOutput", "Objects of this type are created in CallEntry and made available to QML for usage");
    qmlRegisterUncreatableType<AccountEntry>(uri, 0, 1, "AccountEntry", "Objects of this type are created in TelepathyHelper and made available to QML");
    qmlRegisterUncreatableType<USSDManager>(uri, 0, 1, "USSDManager", "Objects of this type are created in AccountEntry and made available to QML");
    qmlRegisterUncreatableType<Protocol>(uri, 0, 1, "ProtocolManager", "Objects of this type are created in ProtocolManager and made available to QML");
    qmlRegisterType<ContactWatcher>(uri, 0, 1, "ContactWatcher");
    qmlRegisterType<PhoneUtils>(uri, 0, 1, "PhoneUtils");
}
