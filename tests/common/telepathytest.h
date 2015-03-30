/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#ifndef TELEPATHYTEST_H
#define TELEPATHYTEST_H

#include <QtCore/QObject>
#include <TelepathyQt/Account>
#include "telepathyhelper.h"

#define DEFAULT_TIMEOUT 15000

class TelepathyTest : public QObject
{
    Q_OBJECT

protected:
    void initialize();
    void doCleanup();

    // helper slots
    void onAccountManagerReady(Tp::PendingOperation *op);
    Tp::AccountPtr addAccount(const QString &manager,
                              const QString &protocol,
                              const QString &displayName,
                              const QVariantMap &parameters = QVariantMap());
    bool removeAccount(const Tp::AccountPtr &account);
    QList<Tp::AccountPtr> accounts() const;

private Q_SLOTS:
    void cleanup();

private:
    Tp::AccountManagerPtr mAccountManager;
    bool mReady;
    QList<Tp::AccountPtr> mAccounts;
};

#endif //TELEPATHYTEST_H
