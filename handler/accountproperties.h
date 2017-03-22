/*
 * Copyright (C) 2017 Canonical, Ltd.
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

#ifndef ACCOUNTPROPERTIES_H
#define ACCOUNTPROPERTIES_H

#include <QObject>

class QSettings;

class AccountProperties : public QObject
{
    Q_OBJECT
public:
    static AccountProperties *instance();

    QMap<QString,QVariantMap> allProperties();
    QVariantMap accountProperties(const QString &accountId);
    void setAccountProperties(const QString &accountId, const QVariantMap &properties);
    QString formatAccountId(const QString &accountId);

protected:
    explicit AccountProperties(QObject *parent = 0);

private:
    QSettings *mSettings;
};

#endif // ACCOUNTPROPERTIES_H
