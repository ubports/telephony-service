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

#ifndef CONTACTDETAIL_H
#define CONTACTDETAIL_H

#include <QString>
#include <QObject>
#include <QContactDetail>

QTCONTACTS_USE_NAMESPACE

class ContactDetail : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int type
               READ type
               NOTIFY typeChanged)
    Q_PROPERTY(QVariant contexts
               READ contexts
               WRITE setContexts
               NOTIFY detailChanged)
    Q_ENUMS(DetailType)
    Q_ENUMS(Context)

public:
    explicit ContactDetail(const QContactDetail &detail = QContactDetail(), QObject *parent = 0);
    virtual ~ContactDetail();

    enum DetailType {
        Name = QContactDetail::TypeName,
        PhoneNumber = QContactDetail::TypePhoneNumber,
        EmailAddress = QContactDetail::TypeEmailAddress,
        Address = QContactDetail::TypeAddress,
        InstantMessaging = QContactDetail::TypeOnlineAccount,
        Unknown = QContactDetail::TypeUndefined
    };

    enum Context {
        ContextHome = QContactDetail::ContextHome,
        ContextWork = QContactDetail::ContextWork,
        ContextOther = QContactDetail::ContextOther
    };

    int type() const;
    void setDetail(const QContactDetail &detail);

    QContactDetail& detail();

    /**
     * For this property and other QStringList properties that are exposed to QML, declare them as QVariant,
     * as the JavaScript array maps to Qt types as a QVariantList and not directly to QStringList.
     */
    QVariant contexts() const;
    void setContexts(const QVariant &contexts);

    static QList<int> intListFromVariant(const QVariant &valueList);
    
Q_SIGNALS:
    void typeChanged();
    void detailChanged();

protected:
    QContactDetail mDetail;
};

#endif // CONTACTDETAIL_H
