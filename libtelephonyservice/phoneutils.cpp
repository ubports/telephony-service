/*
 * Copyright (C) 2012-2015 Canonical, Ltd.
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

#include "phoneutils.h"

#include <phonenumbers/phonenumbermatch.h>
#include <phonenumbers/phonenumbermatcher.h>
#include <phonenumbers/phonenumberutil.h>
#include <phonenumbers/shortnumberutil.h>

#include <QLocale>
#include <QDebug>

QString PhoneUtils::mCountryCode = QString();

PhoneUtils::PhoneUtils(QObject *parent) :
    QObject(parent)
{
}

void PhoneUtils::setCountryCode(const QString &countryCode)
{
    mCountryCode = countryCode;
}

QString PhoneUtils::countryCode()
{
    if (!mCountryCode.isEmpty()) {
        return mCountryCode;
    }

    QString countryCode = QLocale::system().name().split("_").last();
    if (countryCode.size() < 2) {
        // fallback to US if no valid country code was provided, otherwise libphonenumber
        // will fail to parse any numbers
        return QString("US");
    }
    return countryCode;
}

QString PhoneUtils::normalizePhoneNumber(const QString &phoneNumber)
{
    static i18n::phonenumbers::PhoneNumberUtil *phonenumberUtil = i18n::phonenumbers::PhoneNumberUtil::GetInstance();
    if (!isPhoneNumber(phoneNumber)) {
        return phoneNumber;
    }
    std::string number = phoneNumber.toStdString();
    phonenumberUtil->NormalizeDiallableCharsOnly(&number);
    return QString::fromStdString(number);
}

PhoneUtils::PhoneNumberMatchType PhoneUtils::comparePhoneNumbers(const QString &phoneNumberA, const QString &phoneNumberB)
{
    static i18n::phonenumbers::PhoneNumberUtil *phonenumberUtil = i18n::phonenumbers::PhoneNumberUtil::GetInstance();

    // just do a simple string comparison if we are dealing with non phone numbers
    if (!isPhoneNumber(phoneNumberA) || !isPhoneNumber(phoneNumberB)) {
        return phoneNumberA == phoneNumberB ? PhoneUtils::EXACT_MATCH : PhoneUtils::INVALID_NUMBER;
    }
    QString normalizedPhoneNumberA = normalizePhoneNumber(phoneNumberA);
    QString normalizedPhoneNumberB = normalizePhoneNumber(phoneNumberB);

    if (normalizedPhoneNumberA.size() < 7 || normalizedPhoneNumberB.size() < 7) {
        return normalizedPhoneNumberA == normalizedPhoneNumberB ? PhoneUtils::EXACT_MATCH : PhoneUtils::NO_MATCH;
    }
 
    i18n::phonenumbers::PhoneNumberUtil::MatchType match = phonenumberUtil->
            IsNumberMatchWithTwoStrings(phoneNumberA.toStdString(),
                                        phoneNumberB.toStdString());
    return (PhoneNumberMatchType)match;
}

bool PhoneUtils::isPhoneNumber(const QString &phoneNumber)
{
    static i18n::phonenumbers::PhoneNumberUtil *phonenumberUtil = i18n::phonenumbers::PhoneNumberUtil::GetInstance();
    std::string formattedNumber;
    i18n::phonenumbers::PhoneNumber number;
    i18n::phonenumbers::PhoneNumberUtil::ErrorType error;
    error = phonenumberUtil->Parse(phoneNumber.toStdString(), countryCode().toStdString(), &number);

    switch(error) {
    case i18n::phonenumbers::PhoneNumberUtil::INVALID_COUNTRY_CODE_ERROR:
        qWarning() << "Invalid country code for:" << phoneNumber;
        return false;
    case i18n::phonenumbers::PhoneNumberUtil::NOT_A_NUMBER:
        qWarning() << "The phone number is not a valid number:" << phoneNumber;
        return false;
    case i18n::phonenumbers::PhoneNumberUtil::TOO_SHORT_AFTER_IDD:
    case i18n::phonenumbers::PhoneNumberUtil::TOO_SHORT_NSN:
    case i18n::phonenumbers::PhoneNumberUtil::TOO_LONG_NSN:
        qWarning() << "Invalid phone number" << phoneNumber;
        return false;
    default:
        break;
    }
    return true;
}

bool PhoneUtils::isEmergencyNumber(const QString &phoneNumber, const QString &countryCode)
{
    QString finalCode = countryCode;
    if (finalCode.isEmpty()) {
        finalCode = PhoneUtils::countryCode();
    }
    static const i18n::phonenumbers::ShortNumberUtil short_util;
    return short_util.IsEmergencyNumber(normalizePhoneNumber(phoneNumber).toStdString(), finalCode.toStdString());
}
