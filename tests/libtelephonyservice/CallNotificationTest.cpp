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

#include "callnotification.h"

#define INDICATOR_DBUS_SERVICE_NAME "com.canonical.TelephonyServiceIndicator"
#define INDICATOR_DBUS_OBJ_PATH "/com/canonical/TelephonyServiceIndicator"

class CallNotificationTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testClearCallNotification();

private:
    QDBusInterface *mIndicatorInterface;
};

void CallNotificationTest::initTestCase()
{
    mIndicatorInterface = new QDBusInterface(INDICATOR_DBUS_SERVICE_NAME,
                                             INDICATOR_DBUS_OBJ_PATH,
                                             INDICATOR_DBUS_SERVICE_NAME,
                                             QDBusConnection::sessionBus(),
                                             this);
}

void CallNotificationTest::cleanupTestCase()
{
    mIndicatorInterface->deleteLater();
}

void CallNotificationTest::testClearCallNotification()
{
    QSignalSpy clearNotificationSpy(mIndicatorInterface, SIGNAL(ClearCallNotificationRequested(QString, QString)));
    QString targetId("theTargetId");
    QString accountId("theAccountId");
    CallNotification::instance()->clearCallNotification(targetId, accountId);
    QTRY_COMPARE(clearNotificationSpy.count(), 1);
    QCOMPARE(clearNotificationSpy.first()[0].toString(), targetId);
    QCOMPARE(clearNotificationSpy.first()[1].toString(), accountId);
}

QTEST_MAIN(CallNotificationTest)
#include "CallNotificationTest.moc"
