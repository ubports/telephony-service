/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
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

#ifndef APPLICATIONUTILS_H
#define APPLICATIONUTILS_H

#include <QObject>
#include <QUrl>

class ApplicationUtils : public QObject
{
    Q_OBJECT

public:
    static ApplicationUtils *instance();
    static bool checkApplicationRunning(const QString &serviceName);
#ifdef WANT_UI_SERVICES
    static bool openUrl(const QUrl &url);
#endif

private:
    explicit ApplicationUtils(QObject *parent = 0);
};

#endif // APPLICATIONUTILS_H
