#include "phoneutils.h"
#include "phonenumberutils.h"

PhoneUtils::PhoneUtils(QObject *parent) :
    QObject(parent)
{
}

bool PhoneUtils::comparePhoneNumbers(const QString &number1, const QString &number2)
{
    return PhoneNumberUtils::compareLoosely(number1, number2);
}
