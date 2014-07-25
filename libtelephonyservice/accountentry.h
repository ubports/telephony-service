/*
 * Copyright (C) 2013 Canonical, Ltd.
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
    Q_PROPERTY(QString displayName READ displayName NOTIFY displayNameChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QStringList emergencyNumbers READ emergencyNumbers NOTIFY emergencyNumbersChanged)
    Q_PROPERTY(QString voicemailNumber READ voicemailNumber NOTIFY voicemailNumberChanged)

public:
    explicit AccountEntry(const Tp::AccountPtr &account, QObject *parent = 0);
    QString accountId() const;
    QString displayName() const;
    bool connected() const;
    QStringList emergencyNumbers() const;
    QString voicemailNumber() const;
    Tp::AccountPtr account() const;

Q_SIGNALS:
    void accountReady();
    void accountIdChanged();
    void displayNameChanged();
    void connectedChanged();
    void emergencyNumbersChanged();
    void voicemailNumberChanged();

protected Q_SLOTS:
    void initialize();
    void ensureEnabled();
    void ensureConnected();
    void watchSelfContactPresence();

private Q_SLOTS:
    void onConnectionChanged();
    void onEmergencyNumbersChanged(const QStringList &numbers);

private:
    Tp::AccountPtr mAccount;
    QStringList mEmergencyNumbers;
    QString mVoicemailNumber;
    ConnectionInfo mConnectionInfo;
};

#endif // ACCOUNTENTRY_H
