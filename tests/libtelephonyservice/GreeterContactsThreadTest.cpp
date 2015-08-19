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

#include "greetercontacts.h"
#include <QtTest>
#include <QThread>

Q_DECLARE_METATYPE(QtContacts::QContact) // for QVariant's benefit

QTCONTACTS_USE_NAMESPACE

class GreeterContactsThreadTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSingletonAcrossThreads();

};

void GreeterContactsThreadTest::testSingletonAcrossThreads()
{
    // make sure that the ::instance() method works across threads
    QThread thread1;
    thread1.start();
    QTimer timer1;
    GreeterContacts *singleton1 = nullptr;
    timer1.moveToThread(&thread1);
    connect(&timer1, &QTimer::timeout, [&]() {
        singleton1 = GreeterContacts::instance();
        thread1.quit();
    });
    timer1.setSingleShot(true);
    timer1.setInterval(0);

    QThread thread2;
    thread2.start();

    QTimer timer2;
    GreeterContacts *singleton2 = nullptr;
    timer2.moveToThread(&thread2);
    connect(&timer2, &QTimer::timeout, [&]() {
        singleton2 = GreeterContacts::instance();
        thread2.quit();
    });
    timer2.setSingleShot(true);
    timer2.setInterval(0);

    QMetaObject::invokeMethod(&timer1, "start", Qt::QueuedConnection);
    QMetaObject::invokeMethod(&timer2, "start", Qt::QueuedConnection);

    QTRY_VERIFY(thread1.wait());
    QTRY_VERIFY(thread2.wait());

    QVERIFY(singleton1);
    QVERIFY(singleton2);

    QCOMPARE(singleton1, singleton2);
}

QTEST_MAIN(GreeterContactsThreadTest)
#include "GreeterContactsThreadTest.moc"
