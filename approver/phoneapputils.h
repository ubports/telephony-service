/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
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

#ifndef PHONEAPPUTILS_H
#define PHONEAPPUTILS_H

#include <QObject>
#include <QDBusServiceWatcher>

class PhoneAppUtils : public QObject
{
    Q_OBJECT
public:
    static PhoneAppUtils *instance();

    void startPhoneApp();
    bool isApplicationRunning();

Q_SIGNALS:
    void applicationRunningChanged(bool running);

protected Q_SLOTS:
    void onServiceRegistered(const QString &serviceName);
    void onServiceUnregistered(const QString &serviceName);

private:
    explicit PhoneAppUtils(QObject *parent = 0);

    QDBusServiceWatcher mPhoneAppWatcher;
    bool mPhoneAppRunning;

};

#endif // PHONEAPPUTILS_H
