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

#ifndef CONTACTENTRY_H
#define CONTACTENTRY_H

#include <QObject>
#include <QContact>
#include <QQmlListProperty>
#include <QUrl>
#include "contactdetail.h"

QTCONTACTS_USE_NAMESPACE

class ContactName;
class ContactModel;

class ContactEntry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id
               READ idString
               NOTIFY changed)
    Q_PROPERTY(QString displayLabel
               READ displayLabel
               WRITE setDisplayLabel
               NOTIFY changed)
    Q_PROPERTY(bool favorite
               READ favorite
               WRITE setFavorite
               NOTIFY changed)
    Q_PROPERTY(QString initial
               READ initial
               NOTIFY changed)
    Q_PROPERTY(QUrl avatar
               READ avatar
               NOTIFY changed)
    Q_PROPERTY(ContactName *name
               READ name
               NOTIFY changed)
    Q_PROPERTY(bool modified
               READ modified
               NOTIFY modifiedChanged)
    Q_PROPERTY(QQmlListProperty<ContactDetail> addresses
               READ addresses
               NOTIFY changed)
    Q_PROPERTY(QQmlListProperty<ContactDetail> emails
               READ emails
               NOTIFY changed)
    Q_PROPERTY(QQmlListProperty<ContactDetail> onlineAccounts
               READ onlineAccounts
               NOTIFY changed)
    Q_PROPERTY(QQmlListProperty<ContactDetail> phoneNumbers
               READ phoneNumbers
               NOTIFY changed)

public:
    explicit ContactEntry(const QContact &contact = QContact(), ContactModel *parent = 0);

    explicit ContactEntry(const ContactEntry &other);

    QContactId id() const;
    QString idString() const;

    QString displayLabel() const;
    void setDisplayLabel(const QString &value);

    bool favorite() const;
    void setFavorite(bool value);

    QString initial() const;
    QUrl avatar() const;
    ContactName *name() const;

    bool modified() const;
    void setModified(bool value);

    QContact contact() const;
    void setContact(const QContact &contact);

    QQmlListProperty<ContactDetail> addresses();
    QQmlListProperty<ContactDetail> emails();
    QQmlListProperty<ContactDetail> onlineAccounts();
    QQmlListProperty<ContactDetail> phoneNumbers();

    Q_INVOKABLE bool addDetail(ContactDetail *detail);
    Q_INVOKABLE bool removeDetail(ContactDetail *detail);
    Q_INVOKABLE void revertChanges();

    // QQmlListProperty helpers
    static int  detailCount(QQmlListProperty<ContactDetail> *p);
    static ContactDetail* detailAt(QQmlListProperty<ContactDetail> *p, int index);

Q_SIGNALS:
    void changed(ContactEntry *entry);
    void modifiedChanged();

protected:
    void loadDetails();

    template<typename T1, typename T2> void load() {
        Q_FOREACH(const T1 &detail, mContact.details<T1>()) {
            T2 *detailWrapper = new T2(detail, this);
            connect(detailWrapper,
                    SIGNAL(changed()),
                    SLOT(onDetailChanged()));
            mDetails[(ContactDetail::DetailType)detailWrapper->type()].append(detailWrapper);
        }
    }

protected Q_SLOTS:
    void onDetailChanged();

private:
    QContact mContact;
    bool mModified;
    QMap<ContactDetail::DetailType, QList<ContactDetail*> > mDetails;
    ContactModel *mModel;
};

#endif // CONTACTENTRY_H
