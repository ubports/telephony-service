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
#include "telepathytest.h"
#include "messagingmenu.h"
#include "messagingmenumock.h"
#include "telepathyhelper.h"

class MessagingMenuTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void testCallNotificationAdded();
    void testCallNotificationRemoved();
private:
    Tp::AccountPtr mAccount;
};

void MessagingMenuTest::initTestCase()
{
    initialize();

    // just trigger the creation of the mock singleton
    MessagingMenuMock::instance();

    QSignalSpy accountAddedSpy(TelepathyHelper::instance(), SIGNAL(accountAdded(AccountEntry*)));
    mAccount = addAccount("mock", "ofono", "theAccount");
    QTRY_COMPARE(accountAddedSpy.count(), 1);
}

void MessagingMenuTest::cleanupTestCase()
{
    doCleanup();
}

void MessagingMenuTest::cleanup()
{
    // just to prevent the doCleanup() to run on every test
}

void MessagingMenuTest::testCallNotificationAdded()
{
    QString caller("12345");
    QSignalSpy messageAddedSpy(MessagingMenuMock::instance(), SIGNAL(messageAdded(QString,QString,QString,bool)));
    MessagingMenu::instance()->addCall(caller, mAccount->uniqueIdentifier(), QDateTime::currentDateTime());
    QTRY_COMPARE(messageAddedSpy.count(), 1);
    QCOMPARE(messageAddedSpy.first()[1].toString(), caller);
}

void MessagingMenuTest::testCallNotificationRemoved()
{
    QString caller("2345678");
    QSignalSpy messageRemovedSpy(MessagingMenuMock::instance(), SIGNAL(messageRemoved(QString,QString)));
    MessagingMenu::instance()->addCall(caller, mAccount->uniqueIdentifier(), QDateTime::currentDateTime());
    MessagingMenu::instance()->removeCall(caller, mAccount->uniqueIdentifier());
    QCOMPARE(messageRemovedSpy.count(), 1);
    QCOMPARE(messageRemovedSpy.first()[1].toString(), caller);
}

QTEST_MAIN(MessagingMenuTest)
#include "MessagingMenuTest.moc"
