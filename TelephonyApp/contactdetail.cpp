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
    // FIXME: contexts is not a QStringList anymore, it is a QList<int> in Qt5
    return QVariant();//mDetail.contexts();
}

void ContactDetail::setContexts(const QVariant &contexts)
{
    // FIXME: contexts is not a QStringList anymore, it is a QList<int> in Qt5
    //mDetail.setContexts(contexts.toStringList());
    Q_EMIT detailChanged();
}
