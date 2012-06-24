/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include <libnotify/notify.h>

#include "telephonyappapprover.h"
#include <QApplication>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/AbstractClient>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    notify_init("Telephony App Approver");

    Tp::registerTypes();
    Tp::ClientRegistrarPtr registrar = Tp::ClientRegistrar::create();
    Tp::AbstractClientPtr approver = Tp::AbstractClientPtr::dynamicCast(
          Tp::SharedPtr<TelephonyAppApprover>(new TelephonyAppApprover()));
    registrar->registerClient(approver, "TelephonyAppApprover");
    return app.exec();
}
