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
    Q_EMIT detailChanged();
}

QContactDetail& ContactDetail::detail()
{
    return mDetail;
}

QVariant ContactDetail::contexts() const
{
    return QVariant::fromValue(mDetail.contexts());
}

void ContactDetail::setContexts(const QVariant &contexts)
{
    mDetail.setContexts(contexts.value<QList<int> >());
    Q_EMIT detailChanged();
}
