#include "contactname.h"

ContactName::ContactName(const QContactDetail &detail, QObject *parent) :
    ContactDetail(detail, parent)
{
    connect(this,
            SIGNAL(detailChanged()),
            SIGNAL(changed()));
}

int ContactName::type() const
{
    return Name;
}

QString ContactName::customLabel() const
{
    return mDetail.value(QContactName::FieldCustomLabel);
}

void ContactName::setCustomLabel(const QString &value)
{
    if (value != customLabel()) {
        mDetail.setValue(QContactName::FieldCustomLabel, value);
        emit changed();
    }
}

QString ContactName::firstName() const
{
    return mDetail.value(QContactName::FieldFirstName);
}

void ContactName::setFirstName(const QString &value)
{
    if (value != firstName()) {
        mDetail.setValue(QContactName::FieldFirstName, value);
        emit changed();
    }
}

QString ContactName::lastName() const
{
    return mDetail.value(QContactName::FieldLastName);
}

void ContactName::setLastName(const QString &value)
{
    if (value != lastName()) {
        mDetail.setValue(QContactName::FieldLastName, value);
        emit changed();
    }
}

QString ContactName::middleName() const
{
    return mDetail.value(QContactName::FieldMiddleName);
}

void ContactName::setMiddleName(const QString &value)
{
    if (value != middleName()) {
        mDetail.setValue(QContactName::FieldMiddleName, value);
        emit changed();
    }
}

QString ContactName::prefix() const
{
    return mDetail.value(QContactName::FieldPrefix);
}

void ContactName::setPrefix(const QString &value)
{
    if (value != prefix()) {
        mDetail.setValue(QContactName::FieldPrefix, value);
        emit changed();
    }
}

QString ContactName::suffix() const
{
    return mDetail.value(QContactName::FieldSuffix);
}

void ContactName::setSuffix(const QString &value)
{
    if (value != suffix()) {
        mDetail.setValue(QContactName::FieldSuffix, value);
        emit changed();
    }
}
