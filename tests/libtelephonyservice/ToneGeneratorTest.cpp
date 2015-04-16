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

#include "tonegenerator.h"

#define TONEGEN_DBUS_SERVICE_NAME "com.Nokia.Telephony.Tones"
#define TONEGEN_DBUS_OBJ_PATH "/com/Nokia/Telephony/Tones"

class ToneGeneratorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void testSingleTone_data();
    void testSingleTone();
    void testMultipleTones();
    void testWaitingTone();
    void testCallEndedTone();

private:
    QDBusInterface *mToneInterface;
    QSignalSpy *mStartToneSpy;
    QSignalSpy *mStopToneSpy;
};

void ToneGeneratorTest::initTestCase()
{
    mToneInterface = new QDBusInterface(TONEGEN_DBUS_SERVICE_NAME,
                                        TONEGEN_DBUS_OBJ_PATH,
                                        TONEGEN_DBUS_SERVICE_NAME,
                                        QDBusConnection::sessionBus(),
                                        this);

    mStartToneSpy = new QSignalSpy(mToneInterface, SIGNAL(StartEventToneRequested(uint, int, uint)));
    mStopToneSpy = new QSignalSpy(mToneInterface,SIGNAL(StopToneRequested()));
}

void ToneGeneratorTest::cleanupTestCase()
{
    mToneInterface->deleteLater();
    mStartToneSpy->deleteLater();
    mStopToneSpy->deleteLater();
}

void ToneGeneratorTest::cleanup()
{
    mStartToneSpy->clear();
    mStopToneSpy->clear();
}

void ToneGeneratorTest::testSingleTone_data()
{
    QTest::addColumn<uint>("key");

    QTest::newRow("0") << (uint)0;
    QTest::newRow("1") << (uint)1;
    QTest::newRow("2") << (uint)2;
    QTest::newRow("3") << (uint)3;
    QTest::newRow("4") << (uint)4;
    QTest::newRow("5") << (uint)5;
    QTest::newRow("6") << (uint)6;
    QTest::newRow("7") << (uint)7;
    QTest::newRow("8") << (uint)8;
    QTest::newRow("9") << (uint)9;
    QTest::newRow("10") << (uint)10;
    QTest::newRow("11") << (uint)11;
}

void ToneGeneratorTest::testSingleTone()
{
    QFETCH(uint, key);

    ToneGenerator::instance()->playDTMFTone(key);
    QTRY_COMPARE(mStartToneSpy->count(), 1);
    QCOMPARE(mStartToneSpy->first()[0].toUInt(), key);

    QTRY_COMPARE(mStopToneSpy->count(), 1);
}

void ToneGeneratorTest::testMultipleTones()
{
    int count = 12;
    for (uint key = 0; key < count; ++key) {
        ToneGenerator::instance()->playDTMFTone(key);
    }

    QTRY_COMPARE(mStartToneSpy->count(), count);
    QTRY_COMPARE(mStopToneSpy->count(), 1);
}

void ToneGeneratorTest::testWaitingTone()
{
    ToneGenerator::instance()->playWaitingTone();
    QTRY_COMPARE(mStartToneSpy->count(), 1);
    QCOMPARE(mStartToneSpy->first()[0].toUInt(), WAITING_TONE);

    ToneGenerator::instance()->stopWaitingTone();
    QTRY_COMPARE(mStopToneSpy->count(), 1);
}

void ToneGeneratorTest::testCallEndedTone()
{
    ToneGenerator::instance()->playCallEndedTone();

    QTRY_COMPARE(mStartToneSpy->count(), 1);
    QCOMPARE(mStartToneSpy->first()[0].toUInt(), CALL_ENDED_TONE);

    QTRY_COMPARE(mStopToneSpy->count(), 1);
}

QTEST_MAIN(ToneGeneratorTest)
#include "ToneGeneratorTest.moc"
