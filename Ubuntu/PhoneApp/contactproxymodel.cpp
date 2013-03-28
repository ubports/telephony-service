/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "contactproxymodel.h"
#include "contactentry.h"
#include "contactmodel.h"
#include <QDebug>

ContactProxyModel::ContactProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent),
    mFavoriteOnly(false)
{
    setDynamicSortFilter(true);
    setSortLocaleAware(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    sort(0, Qt::AscendingOrder);

    connect(this, SIGNAL(rowsInserted(const QModelIndex&, int, int)), SIGNAL(rowCountChanged()));
    connect(this, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), SIGNAL(rowCountChanged()));
    connect(this, SIGNAL(modelReset()), SIGNAL(rowCountChanged()));
}

QObject *ContactProxyModel::model() const
{
    return sourceModel();
}

void ContactProxyModel::setModel(QObject *value)
{
    ContactModel *model = qobject_cast<ContactModel*>(value);
    if (!model) {
        return;
    }

    setSourceModel(model);
    Q_EMIT modelChanged();
}

QString ContactProxyModel::filterText() const
{
    return mFilterText;
}

void ContactProxyModel::setFilterText(const QString &value)
{
    if (value != mFilterText) {
        mFilterText = value;
        invalidateFilter();
        Q_EMIT filterTextChanged();
    }
}

bool ContactProxyModel::favoriteOnly()
{
    return mFavoriteOnly;
}

void ContactProxyModel::setFavoriteOnly(bool enable)
{
    mFavoriteOnly = enable;
    invalidateFilter();
    Q_EMIT favoriteOnlyChanged();
}

bool ContactProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)

    QObject *object = sourceModel()->index(sourceRow, 0).data(ContactModel::ContactRole).value<QObject*>();
    ContactEntry *entry = qobject_cast<ContactEntry*>(object);

    if (!entry) {
        qWarning() << "ContactRole did not return a valid entry!";
        return false;
    }

    if (mFavoriteOnly) {
        if (!entry->isFavorite())
            return false;
    }

    if (mFilterText.isEmpty()) {
        return true;
    }

    if (entry->displayLabel().indexOf(mFilterText, 0, Qt::CaseInsensitive) >=0) {
        return true;
    }

    // FIXME: filter by the phonenumber
    return false;
}

bool ContactProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    ContactEntry *leftContact = qobject_cast<ContactEntry*>(left.data(ContactModel::ContactRole).value<QObject*>());
    ContactEntry *rightContact = qobject_cast<ContactEntry*>(right.data(ContactModel::ContactRole).value<QObject*>());
    QString leftString = left.data().toString();
    QString rightString = right.data().toString();

    if (leftContact->isFavorite() == rightContact->isFavorite()) {
        return leftString.toLower().localeAwareCompare(rightString.toLower()) < 0;
    }
    return leftContact->isFavorite();
}

