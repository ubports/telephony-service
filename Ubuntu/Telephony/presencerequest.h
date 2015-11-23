/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#ifndef PRESENCEREQUEST_H
#define PRESENCEREQUEST_H

#include <QObject>
#include <QQmlParserStatus>
#include <TelepathyQt/Presence>
#include <TelepathyQt/PendingOperation>

#include "accountentry.h"

class PresenceRequest : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_ENUMS(PresenceType)
    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(QString accountId READ accountId WRITE setAccountId NOTIFY accountIdChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(uint type READ type NOTIFY typeChanged)

public:

    enum PresenceType {
        PresenceTypeUnset = Tp::ConnectionPresenceTypeUnset,
        PresenceTypeOffline = Tp::ConnectionPresenceTypeOffline,
        PresenceTypeAvailable = Tp::ConnectionPresenceTypeAvailable,
        PresenceTypeAway = Tp::ConnectionPresenceTypeAway,
        PresenceTypeExtendedAway = Tp::ConnectionPresenceTypeExtendedAway,
        PresenceTypeHidden = Tp::ConnectionPresenceTypeHidden,
        PresenceTypeBusy = Tp::ConnectionPresenceTypeBusy,
        PresenceTypeUnknown = Tp::ConnectionPresenceTypeUnknown,
        PresenceTypeError = Tp::ConnectionPresenceTypeError
    };

    explicit PresenceRequest(QObject *parent = 0);
    ~PresenceRequest();

    uint type() const;
    QString status() const;
    QString statusMessage() const;

    QString identifier() const;
    void setIdentifier(const QString &identifier);

    QString accountId() const;
    void setAccountId(const QString &accountId);

    void classBegin();
    void componentComplete();

private Q_SLOTS:
    void startPresenceRequest();
    void onPresenceChanged();
    void onContactReceived(Tp::PendingOperation *op);
    void onAccountAdded(AccountEntry *account);

Q_SIGNALS:
    void identifierChanged();
    void accountIdChanged();
    void statusChanged();
    void statusMessageChanged();
    void typeChanged();

private:
    void startSearching();

    QString mIdentifier;
    QString mAccountId;
    bool mCompleted;
    Tp::ContactPtr mContact;
};

#endif // PRESENCEREQUEST_H
