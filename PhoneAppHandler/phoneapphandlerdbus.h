/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Ugo Riboni <ugo.riboni@canonical.com>
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

#ifndef PHONEAPPHANDLERDBUS_H
#define PHONEAPPHANDLERDBUS_H

#include <QtCore/QObject>
#include <QtDBus/QDBusContext>
#include "chatmanager.h"

/**
 * DBus interface for the phone approver
 */
class PhoneAppHandlerDBus : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    PhoneAppHandlerDBus(QObject* parent=0);
    ~PhoneAppHandlerDBus();

    bool connectToBus();

public Q_SLOTS:
    Q_NOREPLY void SendMessage(const QString &number, const QString &message);

Q_SIGNALS:
    void onMessageSent(const QString &number, const QString &message);
};

#endif // PHONEAPPROVERDBUS_H
