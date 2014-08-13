/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#ifndef DISPLAYNAMESETTINGS_H
#define DISPLAYNAMESETTINGS_H

#include <QObject>
#include <QMap>

#include "qgsettings.h"

class DisplayNameSettings : public QObject
{
    Q_OBJECT
public:
    static DisplayNameSettings *instance();

private Q_SLOTS:
    void onAccountsChanged();
    void onSettingsChanged(const QString &key);

private:
    QGSettings mSimNameSettings;
    explicit DisplayNameSettings(QObject *parent = 0);
    QMap<QString,QString> mAccountNames;

};

#endif // DISPLAYNAMESETTINGS_H
