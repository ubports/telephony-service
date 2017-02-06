/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Renato Araujo Oliveira Filho <renato.filho@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#ifndef PHONEUTILS_H
#define PHONEUTILS_H

#include <phonenumbers/phonenumberutil.h>
#include <QObject>

class PhoneUtils : public QObject
{
    Q_OBJECT
    Q_ENUMS(PhoneNumberMatchType)
        
public:
    enum PhoneNumberMatchType {
        INVALID_NUMBER = i18n::phonenumbers::PhoneNumberUtil::INVALID_NUMBER,
        NO_MATCH = i18n::phonenumbers::PhoneNumberUtil::NO_MATCH,
        SHORT_NSN_MATCH = i18n::phonenumbers::PhoneNumberUtil::SHORT_NSN_MATCH,
        NSN_MATCH = i18n::phonenumbers::PhoneNumberUtil::NSN_MATCH,
        EXACT_MATCH = i18n::phonenumbers::PhoneNumberUtil::EXACT_MATCH
    };
    explicit PhoneUtils(QObject *parent = 0);
    Q_INVOKABLE static void setCountryCode(const QString &countryCode);
    Q_INVOKABLE static QString countryCode();
    Q_INVOKABLE static PhoneNumberMatchType comparePhoneNumbers(const QString &number1, const QString &number2);
    Q_INVOKABLE static bool isPhoneNumber(const QString &phoneNumber);
    Q_INVOKABLE static QString normalizePhoneNumber(const QString &phoneNumber);
    Q_INVOKABLE static bool isEmergencyNumber(const QString &phoneNumber, const QString &countryCode = QString());
    Q_INVOKABLE static bool phoneNumberHasCountryCode(const QString &phoneNumber);
    Q_INVOKABLE static QStringList supportedRegions();
    Q_INVOKABLE static QString getFullNumber(const QString &number, const QString &defaultCountryCode, const QString &defaultAreaCode);
private:
    static QString mCountryCode;
    
};

#endif // PHONEUTILS_H
