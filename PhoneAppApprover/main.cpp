/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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


    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    Tp::ClientRegistrarPtr registrar = Tp::ClientRegistrar::create(sessionBus,
                                                                   Tp::AccountFactory::create(sessionBus),
                                                                   Tp::ConnectionFactory::create(sessionBus, Tp::Features()
                                                                                                 << Tp::Connection::FeatureCore
                                                                                                 << Tp::Connection::FeatureSelfContact),
                                                                   Tp::ChannelFactory::create(sessionBus),
                                                                   Tp::ContactFactory::create(Tp::Features()
                                                                                              << Tp::Contact::FeatureAlias
                                                                                              << Tp::Contact::FeatureAvatarData
                                                                                              << Tp::Contact::FeatureAvatarToken));

    // register the approver
    Tp::AbstractClientPtr approver = Tp::AbstractClientPtr::dynamicCast(
          Tp::SharedPtr<PhoneAppApprover>(new PhoneAppApprover()));
    registrar->registerClient(approver, "PhoneAppApprover");

    // and the observer
    Tp::AbstractClientPtr observer = Tp::AbstractClientPtr::dynamicCast(
          Tp::SharedPtr<TextChannelObserver>(new TextChannelObserver()));
    registrar->registerClient(observer, "PhoneAppIndicatorObserver");

    // we don't need to call anything on the indicator, it will work by itself
    VoiceMailIndicator voiceMailIndicator;
    Q_UNUSED(voiceMailIndicator);

    // create the contact model so that contacts are already loaded
    ContactModel::instance();

    return app.exec();
}
