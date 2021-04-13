/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *  Renato Araujo Oliveira Filho <renato.filho@canonical.com>
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

#ifndef TELEPHONY_PHONEUTILS_H
#define TELEPHONY_PHONEUTILS_H

#include <QtCore/QObject>
#include <QtCore/QEvent>

class PhoneUtils : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString defaultRegion READ defaultRegion NOTIFY defaultRegionChanged)
public:
    enum PhoneNumberFormat {
        E164 = 0,
        International,
        National,
        RFC3966,
        Auto
    };

    PhoneUtils(QObject *parent = 0);
    ~PhoneUtils();

    QString defaultRegion() const;

    Q_INVOKABLE QStringList matchInText(const QString& text, const QString &defaultRegion = QString());

    Q_INVOKABLE QString format(const QString &phoneNumber, const QString &defaultRegion = QString(), PhoneNumberFormat format = Auto);

    Q_INVOKABLE int getCountryCodePrefix(const QString &regionCode) const;

    virtual bool event(QEvent *event);

Q_SIGNALS:
    void defaultRegionChanged();
};

#endif
