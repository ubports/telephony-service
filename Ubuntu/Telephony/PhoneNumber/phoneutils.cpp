#include "phoneutils.h"

#include <QtCore/QDebug>
#include <phonenumbers/phonenumberutil.h>



PhoneUtils::PhoneUtils(QObject *parent)
    : QObject(parent)
{
}

PhoneUtils::~PhoneUtils()
{
}

QString PhoneUtils::format(const QString &phoneNumber, const QString &defaultRegion,  PhoneUtils::PhoneNumberFormat format)
{
    i18n::phonenumbers::PhoneNumberUtil *phonenumberUtil = i18n::phonenumbers::PhoneNumberUtil::GetInstance();

    i18n::phonenumbers::PhoneNumber number;
    i18n::phonenumbers::PhoneNumberUtil::ErrorType error;
    error = phonenumberUtil->Parse(phoneNumber.toStdString(), defaultRegion.toStdString(), &number);

    switch(error) {
    case i18n::phonenumbers::PhoneNumberUtil::INVALID_COUNTRY_CODE_ERROR:
        qWarning() << "Invalid coutry code for:" << phoneNumber;
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

    std::string formattedNumber;
    i18n::phonenumbers::PhoneNumberUtil::PhoneNumberFormat pNFormat;
    if (format == PhoneUtils::Auto) {
        if (phoneNumber.startsWith("+")) {
            pNFormat = i18n::phonenumbers::PhoneNumberUtil::INTERNATIONAL;
        } else {
            pNFormat = i18n::phonenumbers::PhoneNumberUtil::NATIONAL;
        }
    } else {
        pNFormat = static_cast<i18n::phonenumbers::PhoneNumberUtil::PhoneNumberFormat>(format);
    }

    phonenumberUtil->Format(number,
                            pNFormat,
                            &formattedNumber);
    return QString::fromStdString(formattedNumber);
}


