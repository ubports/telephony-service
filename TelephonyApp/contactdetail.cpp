#include "contactdetail.h"

ContactDetail::ContactDetail(const QContactDetail &detail, QObject *parent) :
    QObject(parent), mDetail(detail)
{
}

int ContactDetail::type() const
{
    return Unknown;
}

void ContactDetail::setDetail(const QContactDetail &detail)
{
    mDetail = detail;
    emit detailChanged();
}

QStringList ContactDetail::contexts() const
{
    return mDetail.contexts();
}

void ContactDetail::setContexts(const QStringList &contexts)
{
    mDetail.setContexts(contexts);
    emit detailChanged();
}
