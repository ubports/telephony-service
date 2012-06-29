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

QContactDetail& ContactDetail::detail()
{
    return mDetail;
}

QVariant ContactDetail::contexts() const
{
    return mDetail.contexts();
}

void ContactDetail::setContexts(const QVariant &contexts)
{
    mDetail.setContexts(contexts.toStringList());
    emit detailChanged();
}

QString ContactDetail::definitionName() const
{
    return mDetail.definitionName();
}
