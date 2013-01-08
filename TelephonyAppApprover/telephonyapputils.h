/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TELEPHONYAPPUTILS_H
#define TELEPHONYAPPUTILS_H

#include <QObject>
#include <QDBusServiceWatcher>

class TelephonyAppUtils : public QObject
{
    Q_OBJECT
public:
    static TelephonyAppUtils *instance();

    void startTelephonyApp();
    bool isApplicationRunning();

Q_SIGNALS:
    void applicationRunningChanged(bool running);

protected Q_SLOTS:
    void onServiceRegistered(const QString &serviceName);
    void onServiceUnregistered(const QString &serviceName);

private:
    explicit TelephonyAppUtils(QObject *parent = 0);

    QDBusServiceWatcher mTelephonyAppWatcher;
    bool mTelephonyAppRunning;
    
};

#endif // TELEPHONYAPPUTILS_H
