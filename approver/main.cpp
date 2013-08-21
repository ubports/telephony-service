/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
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

#include <libnotify/notify.h>

#include "applicationutils.h"
#include "approver.h"
#include "telepathyhelper.h"
#include "textchannelobserver.h"
#include "voicemailindicator.h"
#include <QCoreApplication>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/AbstractClient>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Contact>

namespace C {
#include <libintl.h>
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    C::bindtextdomain( "telephony-service", "/usr/share/locale" );
    C::textdomain("telephony-service");

    notify_init(C::gettext("Telephony Service Approver"));

    Tp::registerTypes();

    // check if there is already an instance of the approver running
    if (ApplicationUtils::checkApplicationRunning(TP_QT_IFACE_CLIENT + ".TelephonyServiceApprover")) {
        qDebug() << "Found another instance of the approver. Quitting.";
        return 1;
    }

    // register the approver
    Approver *approver = new Approver();
    TelepathyHelper::instance()->registerClient(approver, "TelephonyServiceApprover");

    // and the observer
    TelepathyHelper::instance()->registerChannelObserver("TelephonyServiceObserver");

    // Connect the textObserver to the channel observer in TelepathyHelper
    TextChannelObserver *textObserver = new TextChannelObserver();
    QObject::connect(TelepathyHelper::instance()->channelObserver(), SIGNAL(textChannelAvailable(Tp::TextChannelPtr)),
                     textObserver, SLOT(onTextChannelAvailable(Tp::TextChannelPtr)));

    // we don't need to call anything on the indicator, it will work by itself
    VoiceMailIndicator voiceMailIndicator;
    Q_UNUSED(voiceMailIndicator);

    return app.exec();
}
