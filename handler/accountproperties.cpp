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

#include "accountproperties.h"
#include "telepathyhelper.h"
#include <QSettings>

#define SETTINGS_DOMAIN "com.canonical.TelephonyServiceHandler"

AccountProperties *AccountProperties::instance()
{
    static AccountProperties *self = new AccountProperties();
    return self;
}

QMap<QString, QVariantMap> AccountProperties::allProperties()
{
    QMap<QString,QVariantMap> props;
    for (auto accountId : TelepathyHelper::instance()->accountIds()) {
        props[accountId] = accountProperties(accountId);
    }
}

QVariantMap AccountProperties::accountProperties(const QString &accountId)
{
    QVariantMap props;
    mSettings->beginGroup(formatAccountId(accountId));
    for (auto key : mSettings->allKeys()) {
        props[key] = mSettings->value(key);
    }
    mSettings->endGroup();
    return props;
}

void AccountProperties::setAccountProperties(const QString &accountId, const QVariantMap &properties)
{
    mSettings->beginGroup(formatAccountId(accountId));
    for (auto key : properties.keys()) {
        mSettings->setValue(key, properties[key]);
    }
    mSettings->endGroup();
}

QString AccountProperties::formatAccountId(const QString &accountId)
{
    return QUrl::toPercentEncoding(accountId);
}

AccountProperties::AccountProperties(QObject *parent)
: QObject(parent),
  mSettings(new QSettings(SETTINGS_DOMAIN, QString(), parent))
{
}
