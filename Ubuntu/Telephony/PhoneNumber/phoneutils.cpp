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

#include "phoneutils.h"

#include <QtCore/QDebug>
#include <phonenumbers/phonenumbermatch.h>
#include <phonenumbers/phonenumbermatcher.h>
#include <phonenumbers/phonenumberutil.h>


PhoneUtils::PhoneUtils(QObject *parent)
    : QObject(parent)
{
}

PhoneUtils::~PhoneUtils()
{
}

QString PhoneUtils::defaultRegion() const
{
    QString locale = QLocale::system().name();
    return locale.split("_").last();
}

/*!
  From a locale code, e.g FR, retrieve the phone prefix: "33"
  If regionCode is empty, fallback to US
*/
int PhoneUtils::getCountryCodePrefix(const QString &regionCode) const
{
    QString locale = regionCode.isEmpty() ? QString("US") : regionCode;
    return i18n::phonenumbers::PhoneNumberUtil::GetInstance()->GetCountryCodeForRegion(locale.toStdString());
}

QString PhoneUtils::format(const QString &phoneNumber, const QString &defaultRegion,  PhoneUtils::PhoneNumberFormat format)
{
    std::string formattedNumber;
    i18n::phonenumbers::PhoneNumberUtil::PhoneNumberFormat pNFormat;
    if (format == PhoneUtils::Auto) {
        // skip if it is a special number or a command
        if (phoneNumber.startsWith("#") || phoneNumber.startsWith("*")) {
            return phoneNumber;
        } else if (phoneNumber.startsWith("+")) {
            pNFormat = i18n::phonenumbers::PhoneNumberUtil::INTERNATIONAL;
        } else {
            pNFormat = i18n::phonenumbers::PhoneNumberUtil::NATIONAL;
        }
    } else {
        pNFormat = static_cast<i18n::phonenumbers::PhoneNumberUtil::PhoneNumberFormat>(format);
    }


    i18n::phonenumbers::PhoneNumberUtil *phonenumberUtil = i18n::phonenumbers::PhoneNumberUtil::GetInstance();

    i18n::phonenumbers::PhoneNumber number;
    i18n::phonenumbers::PhoneNumberUtil::ErrorType error;
    QString region = defaultRegion.isEmpty() ? this->defaultRegion() : defaultRegion;
    error = phonenumberUtil->Parse(phoneNumber.toStdString(), region.toStdString(), &number);

    switch(error) {
    case i18n::phonenumbers::PhoneNumberUtil::INVALID_COUNTRY_CODE_ERROR:
        qWarning() << "Invalid country code for:" << phoneNumber;
        return "";
    case i18n::phonenumbers::PhoneNumberUtil::NOT_A_NUMBER:
        qWarning() << "The phone number is not a valid number:" << phoneNumber;
        return "";
    case i18n::phonenumbers::PhoneNumberUtil::TOO_SHORT_AFTER_IDD:
    case i18n::phonenumbers::PhoneNumberUtil::TOO_SHORT_NSN:
    case i18n::phonenumbers::PhoneNumberUtil::TOO_LONG_NSN:
        qWarning() << "Invalid phone number" << phoneNumber;
        return "";
    default:
        break;
    }


    phonenumberUtil->Format(number,
                            pNFormat,
                            &formattedNumber);
    return QString::fromStdString(formattedNumber);
}

bool PhoneUtils::event(QEvent *event)
{
    if (event->type() == QEvent::LocaleChange) {
        Q_EMIT defaultRegionChanged();
    }
    return QObject::event(event);
}

QStringList PhoneUtils::matchInText(const QString& text, const QString &defaultRegion)
{
    if (text.isEmpty()) {
        return QStringList();
    }

    QString region = defaultRegion.isEmpty() ? this->defaultRegion() : defaultRegion;

    QStringList matches;
    i18n::phonenumbers::PhoneNumberMatcher matcher(text.toStdString(), region.toStdString());
    if (matcher.HasNext()) {
        i18n::phonenumbers::PhoneNumberMatch match;
        matcher.Next(&match);
        matches.append(QString::fromStdString(match.raw_string()));
    }

    return matches;
}

