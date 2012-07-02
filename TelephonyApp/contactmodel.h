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

#ifndef CONTACTMODEL_H
#define CONTACTMODEL_H

#include <QAbstractListModel>
#include <QContactManager>

class ContactEntry;

using namespace QtMobility;

class ContactModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ModelRoles {
        ContactRole = Qt::UserRole
    };

    static ContactModel *instance();

    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    Q_INVOKABLE ContactEntry *contactFromId(const QString &guid);
    Q_INVOKABLE ContactEntry *contactFromCustomId(const QString &customId);
    Q_INVOKABLE ContactEntry *contactFromPhoneNumber(const QString &phoneNumber);
    Q_INVOKABLE void saveContact(ContactEntry *entry);
    Q_INVOKABLE void loadContactFromId(const QString &guid);
    Q_INVOKABLE void removeContact(ContactEntry *entry);

    void updateContact(ContactEntry *entry);

Q_SIGNALS:
    void contactLoaded(ContactEntry *contact);
    void contactAdded(ContactEntry *contact);
    void contactChanged(ContactEntry *contact);
    void contactRemoved(const QString &contactId);

protected:
    void addContacts(const QList<QContact> &contacts);
    void removeContactFromModel(ContactEntry *entry);

protected Q_SLOTS:
    void onContactsAdded(QList<QContactLocalId> ids);
    void onContactsChanged(QList<QContactLocalId> ids);
    void onContactsRemoved(QList<QContactLocalId> ids);
    void onContactEntryChanged(ContactEntry *entry);
    void onContactSaved();
    void onContactRemoved();

private:
    explicit ContactModel(QObject *parent = 0);

    QContactManager *mContactManager;
    QList<ContactEntry*> mContactEntries;
    QString mPendingId;
};

#endif // CONTACTMODEL_H
