/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *    Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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

#include "applicationutils.h"
#include "callhandler.h"
#include "displaynamesettings.h"
#include "handler.h"
#include "handlerdbus.h"
#include "telepathyhelper.h"
#include "texthandler.h"
#include <QCoreApplication>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/AbstractClient>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Contact>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("telephony-service-handler");

    Tp::registerTypes();

    // check if there is already an instance of the handler running
    if (ApplicationUtils::checkApplicationRunning(TP_QT_IFACE_CLIENT + ".TelephonyServiceHandler")) {
        qDebug() << "Found another instance of the handler. Quitting.";
        return 1;
    }

    Handler *handler = new Handler();
    TelepathyHelper::instance()->registerClient(handler, "TelephonyServiceHandler");

    QObject::connect(handler, SIGNAL(callChannelAvailable(Tp::CallChannelPtr)),
                     CallHandler::instance(), SLOT(onCallChannelAvailable(Tp::CallChannelPtr)));
    QObject::connect(handler, SIGNAL(textChannelAvailable(Tp::TextChannelPtr)),
                     TextHandler::instance(), SLOT(onTextChannelAvailable(Tp::TextChannelPtr)));

    HandlerDBus dbus;

    QObject::connect(TelepathyHelper::instance(), SIGNAL(setupReady()),
                     &dbus, SLOT(connectToBus()));

    // instanciate the display name settings singleton, it will work by itself
    DisplayNameSettings::instance();

    return app.exec();
}
