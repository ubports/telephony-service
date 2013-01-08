/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
