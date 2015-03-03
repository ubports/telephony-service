/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ACCOUNTENTRY_H
#define ACCOUNTENTRY_H

#include <QObject>
#include <TelepathyQt/Account>


typedef struct {
    QString busName;
    QString objectPath;
} ConnectionInfo;

class AccountEntry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString accountId READ accountId NOTIFY accountIdChanged)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString selfContactId READ selfContactId NOTIFY selfContactIdChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_ENUMS(AccountType)
    friend class AccountEntryFactory;

public:
    enum AccountType {
        PhoneAccount,
        GenericAccount
    };

    QString accountId() const;
    bool active() const;
    QString displayName() const;
    QString status() const;
    QString statusMessage() const;
    QString selfContactId() const;
    void setDisplayName(const QString &name);
    virtual bool connected() const;
    Tp::AccountPtr account() const;
    virtual AccountType type() const;

Q_SIGNALS:
    void accountReady();
    void accountIdChanged();
    void activeChanged();
    void displayNameChanged();
    void statusChanged();
    void statusMessageChanged();
    void selfContactIdChanged();
    void connectedChanged();
    void removed();

protected Q_SLOTS:
    virtual void initialize();
    virtual void ensureEnabled();
    virtual void ensureConnected();
    virtual void watchSelfContactPresence();
    virtual void onConnectionChanged();
    virtual void onSelfHandleChanged(uint handle);

protected:
    explicit AccountEntry(const Tp::AccountPtr &account, QObject *parent = 0);

    Tp::AccountPtr mAccount;
    ConnectionInfo mConnectionInfo;
};

#endif // ACCOUNTENTRY_H
