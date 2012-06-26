/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Ugo Riboni <ugo.riboni@canonical.com>
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

#ifndef TELEPHONYAPPDBUS_H
#define TELEPHONYAPPDBUS_H

#include <QtCore/QObject>
#include <QtDBus/QDBusContext>

/**
 * DBus interface for the telephony app
 */
class TelephonyAppDBus : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    TelephonyAppDBus(QObject* parent=0);
    ~TelephonyAppDBus();

    bool connectToBus();

public Q_SLOTS:
    Q_NOREPLY void ShowMessages();

Q_SIGNALS:
    void showMessagesRequested();
};

#endif // TELEPHONYAPPDBUS_H
