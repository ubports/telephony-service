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

class Protocol;

typedef struct {
    QString busName;
    QString objectPath;
} ConnectionInfo;

class AccountEntry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(AccountType type READ type CONSTANT)
    Q_PROPERTY(QString accountId READ accountId NOTIFY accountIdChanged)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString selfContactId READ selfContactId NOTIFY selfContactIdChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QStringList addressableVCardFields READ addressableVCardFields NOTIFY addressableVCardFieldsChanged)
    Q_PROPERTY(Protocol* protocolInfo READ protocolInfo CONSTANT)
    Q_ENUMS(AccountType)
    friend class AccountEntryFactory;

public:
    enum AccountType {
        PhoneAccount,
        MultimediaAccount,
        GenericAccount
    };

    bool ready() const;
    QString accountId() const;
    QString displayName() const;
    QString status() const;
    QString statusMessage() const;
    QString selfContactId() const;
    void setDisplayName(const QString &name);
    Tp::AccountPtr account() const;
    virtual AccountType type() const;
    virtual QStringList addressableVCardFields() const;
    virtual bool compareIds(const QString &first, const QString &second) const;
    virtual bool active() const;
    virtual bool connected() const;
 
    Protocol *protocolInfo() const;

Q_SIGNALS:
    void accountReady();
    void accountIdChanged();
    void activeChanged();
    void displayNameChanged();
    void statusChanged();
    void statusMessageChanged();
    void selfContactIdChanged();
    void connectedChanged();
    void addressableVCardFieldsChanged();
    void removed();
    void connectionStatusChanged(Tp::ConnectionStatus status);

protected Q_SLOTS:
    virtual void initialize();
    virtual void watchSelfContactPresence();
    virtual void onConnectionChanged(Tp::ConnectionPtr connection);
    virtual void onSelfHandleChanged(uint handle);

protected:
    explicit AccountEntry(const Tp::AccountPtr &account, QObject *parent = 0);

    Tp::AccountPtr mAccount;
    ConnectionInfo mConnectionInfo;
    bool mReady;
    Protocol *mProtocol;
};

#endif // ACCOUNTENTRY_H
