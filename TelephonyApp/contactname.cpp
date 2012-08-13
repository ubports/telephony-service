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
        Q_EMIT changed();
        Q_EMIT customLabelChanged();
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
        Q_EMIT changed();
        Q_EMIT firstNameChanged();
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
        Q_EMIT changed();
        Q_EMIT lastNameChanged();
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
        Q_EMIT changed();
        Q_EMIT middleNameChanged();
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
        Q_EMIT changed();
        Q_EMIT prefixChanged();
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
        Q_EMIT changed();
        Q_EMIT suffixChanged();
    }
}
