#include "contactemailaddress.h"
#include <QContactEmailAddress>

ContactEmailAddress::ContactEmailAddress(const QContactDetail &detail, QObject *parent) :
    ContactDetail(detail, parent)
{
    connect(this,
            SIGNAL(detailChanged()),
            SIGNAL(changed()));
}

int ContactEmailAddress::type() const
{
    return EmailAddress;
}

QString ContactEmailAddress::emailAddress() const
{
    return mDetail.value(QContactEmailAddress::FieldEmailAddress);
}

void ContactEmailAddress::setEmailAddress(const QString &value)
{
    if (value != emailAddress()) {
        mDetail.setValue(QContactEmailAddress::FieldEmailAddress, value);
        emit changed();
    }
}
