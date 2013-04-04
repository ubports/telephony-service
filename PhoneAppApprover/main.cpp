/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libnotify/notify.h>

#include "phoneappapprover.h"
#include "telepathyhelper.h"
#include "textchannelobserver.h"
#include "voicemailindicator.h"
#include "contactmodel.h"
#include <QCoreApplication>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/AbstractClient>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Contact>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    notify_init("Phone App Approver");

    Tp::registerTypes();

    TelepathyHelper::instance()->registerClients();

    // Connect the textObserver to the channel observer in TelepathyHelper
    TextChannelObserver *textObserver = new TextChannelObserver();
    QObject::connect(TelepathyHelper::instance()->channelObserver(), SIGNAL(textChannelAvailable(Tp::TextChannelPtr)),
                     textObserver, SLOT(onTextChannelAvailable(Tp::TextChannelPtr)));

    // register the approver
    PhoneAppApprover *approver = new PhoneAppApprover();
    TelepathyHelper::instance()->registerClient(approver, "PhoneAppApprover");

    // we don't need to call anything on the indicator, it will work by itself
    VoiceMailIndicator voiceMailIndicator;
    Q_UNUSED(voiceMailIndicator);

    // create the contact model so that contacts are already loaded
    ContactModel::instance();

    return app.exec();
}
