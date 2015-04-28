/*
 * Copyright (C) 2012-2015 Canonical, Ltd.
 *
 * Authors:
 *  Ugo Riboni <ugo.riboni@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *  Gustavo Pichorim Boiko <gustavo.boiko@gmail.com>
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

#ifndef INDICATORDBUS_H
#define INDICATORDBUS_H

#include <QtCore/QObject>
#include <QtDBus/QDBusContext>
#include "chatmanager.h"

/**
 * DBus interface for the phone approver
 */
class IndicatorDBus : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    IndicatorDBus(QObject* parent=0);
    ~IndicatorDBus();

    bool connectToBus();

public Q_SLOTS:
    Q_NOREPLY void ClearNotifications();

Q_SIGNALS:
    void clearNotificationsRequested();

};

#endif // IndicatorDBus_H
