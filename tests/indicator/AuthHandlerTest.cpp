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

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QDBusMetaType>
#include "telepathytest.h"
#include "notificationmenu.h"
#include "authhandler.h"
#include "mockcontroller.h"

class AuthHandlerTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testNotificationDisplayedForFailure();

private:
    AuthHandler mAuthHandler;
    Tp::AccountPtr mAccount;
    MockController *mMockController;
};

void AuthHandlerTest::initTestCase()
{
    initialize();
}

void AuthHandlerTest::init()
{
    mAccount = addAccount("mock", "mock", "theAccount");
    mMockController = new MockController("mock", this);
}

void AuthHandlerTest::cleanup()
{
    doCleanup();
    mMockController->deleteLater();
}

void AuthHandlerTest::testNotificationDisplayedForFailure()
{
    QDBusInterface notificationMock("org.freedesktop.Notifications", "/org/freedesktop/Notifications");
    QSignalSpy notificationSpy(&notificationMock, SIGNAL(MockNotificationReceived(QString, uint, QString, QString, QString, QStringList, QVariantMap, int)));

    mMockController->SimulateAuthFailure();
    TRY_COMPARE(notificationSpy.count(), 1);

    // now just check that the actions are there
    QStringList actions = notificationSpy.first()[5].toStringList();
    QCOMPARE(actions.count(), 4);
    QCOMPARE(actions[0], QString("yes_id"));
    QCOMPARE(actions[2], QString("no_id"));
}

QTEST_MAIN(AuthHandlerTest)
#include "AuthHandlerTest.moc"
