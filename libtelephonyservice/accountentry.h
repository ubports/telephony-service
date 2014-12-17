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
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QStringList emergencyNumbers READ emergencyNumbers NOTIFY emergencyNumbersChanged)
    Q_PROPERTY(QString voicemailNumber READ voicemailNumber NOTIFY voicemailNumberChanged)
    Q_PROPERTY(uint voicemailCount READ voicemailCount NOTIFY voicemailCountChanged)
    Q_PROPERTY(bool voicemailIndicator READ voicemailIndicator NOTIFY voicemailIndicatorChanged)
    Q_PROPERTY(QString networkName READ networkName NOTIFY networkNameChanged)
    Q_PROPERTY(bool emergencyCallsAvailable READ emergencyCallsAvailable NOTIFY emergencyCallsAvailableChanged)
    Q_PROPERTY(bool simLocked READ simLocked NOTIFY simLockedChanged)

public:
    explicit AccountEntry(const Tp::AccountPtr &account, QObject *parent = 0);
    QString accountId() const;
    QString displayName() const;
    QString networkName() const;
    bool simLocked() const;
    void setDisplayName(const QString &name);
    bool connected() const;
    QStringList emergencyNumbers() const;
    QString voicemailNumber() const;
    uint voicemailCount() const;
    bool voicemailIndicator() const;
    Tp::AccountPtr account() const;
    bool emergencyCallsAvailable() const;

Q_SIGNALS:
    void accountReady();
    void accountIdChanged();
    void displayNameChanged();
    void networkNameChanged();
    void simLockedChanged();
    void connectedChanged();
    void emergencyNumbersChanged();
    void voicemailNumberChanged();
    void voicemailCountChanged();
    void voicemailIndicatorChanged();
    void emergencyCallsAvailableChanged();
    void removed();

protected Q_SLOTS:
    void initialize();
    void ensureEnabled();
    void ensureConnected();
    void watchSelfContactPresence();

private Q_SLOTS:
    void onConnectionChanged();
    void onEmergencyNumbersChanged(const QStringList &numbers);
    void onVoicemailNumberChanged(const QString &number);
    void onVoicemailCountChanged(uint count);
    void onVoicemailIndicatorChanged(bool visible);

private:
    Tp::AccountPtr mAccount;
    QStringList mEmergencyNumbers;
    QString mVoicemailNumber;
    uint mVoicemailCount;
    bool mVoicemailIndicator;
    ConnectionInfo mConnectionInfo;
};

#endif // ACCOUNTENTRY_H
