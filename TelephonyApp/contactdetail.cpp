#include "contactdetail.h"

ContactDetail::ContactDetail(const QContactDetail &detail, QObject *parent) :
    QObject(parent), mDetail(detail)
{
}

int ContactDetail::type() const
{
    return mDetail.type();
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
    return mDetail.value<QVariant>(QContactDetail::FieldContext);
}

void ContactDetail::setContexts(const QVariant &contexts)
{
    mDetail.setValue(QContactDetail::FieldContext, contexts);
    Q_EMIT detailChanged();
}
