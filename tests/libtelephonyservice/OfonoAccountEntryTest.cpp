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
#include "ofonoaccountentry.h"
#include "telepathyhelper.h"
#include "mockcontroller.h"

#define DEFAULT_TIMEOUT 15000

class OfonoAccountEntryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testConnected();
    void testCompareIds_data();
    void testCompareIds();
    void testEmergencyNumbers();
    void testSerial();
    void testVoicemailIndicator();
    void testVoicemailNumber();
    void testVoicemailCount();
    void testSimLocked();
    void testEmergencyCallsAvailable_data();
    void testEmergencyCallsAvailable();

private:
    OfonoAccountEntry *mAccount;
    Tp::AccountPtr mTpAccount;
    MockController *mMockController;
};

void OfonoAccountEntryTest::initTestCase()
{
    Tp::registerTypes();

    QSignalSpy spy(TelepathyHelper::instance(), SIGNAL(setupReady()));
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, DEFAULT_TIMEOUT);
    QTRY_VERIFY_WITH_TIMEOUT(TelepathyHelper::instance()->connected(), DEFAULT_TIMEOUT);

    mAccount = qobject_cast<OfonoAccountEntry*>(TelepathyHelper::instance()->accountForId("mock/ofono/account0"));
    QVERIFY(mAccount);

    mTpAccount = mAccount->account();
    QVERIFY(!mTpAccount.isNull());
    QTRY_VERIFY(mTpAccount->isReady(Tp::Account::FeatureCore));

    // wait for the connection to appear
    QTRY_VERIFY(!mTpAccount->connection().isNull());

    // create the mock controller
    mMockController = new MockController("ofono", this);
}

void OfonoAccountEntryTest::testConnected()
{
    // right now the ofono account connection status behave exactly like the generic class,
    // but as the code path is different, test it again
    QSignalSpy connectedChangedSpy(mAccount, SIGNAL(connectedChanged()));

    // the mock account is enabled/connected by default, so make sure it is like that
    QVERIFY(mAccount->connected());

    // now set the account offline and see if the active flag changes correctly
    mMockController->setOnline(false);
    QTRY_VERIFY(!mAccount->connected());
    QCOMPARE(connectedChangedSpy.count(), 1);

    // now re-enable the account and check that the entry is updated
    connectedChangedSpy.clear();
    mMockController->setOnline(true);
    QTRY_VERIFY(mAccount->connected());
    QCOMPARE(connectedChangedSpy.count(), 1);
}

void OfonoAccountEntryTest::testCompareIds_data()
{
    QTest::addColumn<QString>("first");
    QTest::addColumn<QString>("second");
    QTest::addColumn<bool>("expectedResult");

    QTest::newRow("identical values") << "1234567" << "1234567" << true;
    QTest::newRow("case difference") << "TestId" << "testid" << false;
    QTest::newRow("phone prefix") << "1234567" << "1231234567" << true;
}

void OfonoAccountEntryTest::testCompareIds()
{
    QFETCH(QString, first);
    QFETCH(QString, second);
    QFETCH(bool, expectedResult);

    QCOMPARE(mAccount->compareIds(first, second), expectedResult);
}

void OfonoAccountEntryTest::testEmergencyNumbers()
{
    QSignalSpy emergencyNumbersChangedSpy(mAccount, SIGNAL(emergencyNumbersChanged()));

    // check that the list is not empty at startup
    QVERIFY(!mAccount->emergencyNumbers().isEmpty());

    QStringList numbers;
    numbers << "111" << "190" << "911";
    qSort(numbers);
    mMockController->setEmergencyNumbers(numbers);
    QTRY_COMPARE(emergencyNumbersChangedSpy.count(), 1);

    QStringList emergencyNumbers = mAccount->emergencyNumbers();
    qSort(emergencyNumbers);

    QCOMPARE(emergencyNumbers, numbers);
}

void OfonoAccountEntryTest::testSerial()
{
    QCOMPARE(mAccount->serial(), mMockController->serial());
}

void OfonoAccountEntryTest::testVoicemailIndicator()
{
    // voicemail indicator is off by default on the mock CM
    QVERIFY(!mAccount->voicemailIndicator());

    QSignalSpy voiceMailIndicatorSpy(mAccount, SIGNAL(voicemailIndicatorChanged()));

    // set to true
    mMockController->setVoicemailIndicator(true);
    QTRY_COMPARE(voiceMailIndicatorSpy.count(), 1);
    QVERIFY(mAccount->voicemailIndicator());

    // and set back to false
    voiceMailIndicatorSpy.clear();
    mMockController->setVoicemailIndicator(false);
    QTRY_COMPARE(voiceMailIndicatorSpy.count(), 1);
    QVERIFY(!mAccount->voicemailIndicator());
}

void OfonoAccountEntryTest::testVoicemailNumber()
{
    QSignalSpy voicemailNumberSpy(mAccount, SIGNAL(voicemailNumberChanged()));

    // check that the number is not empty at startup
    QVERIFY(!mAccount->voicemailNumber().isEmpty());

    // try changing the number
    QString number("12345");
    mMockController->setVoicemailNumber(number);
    QTRY_COMPARE(voicemailNumberSpy.count(), 1);
    QCOMPARE(mAccount->voicemailNumber(), number);
}

void OfonoAccountEntryTest::testVoicemailCount()
{
    QSignalSpy voicemailCountSpy(mAccount, SIGNAL(voicemailCountChanged()));

    // check that the count is zero at startup
    QCOMPARE((int)mAccount->voicemailCount(), 0);

    // set it to a bigger value
    int count = 10;
    mMockController->setVoicemailCount(count);
    QTRY_COMPARE(voicemailCountSpy.count(), 1);
    QCOMPARE((int)mAccount->voicemailCount(), count);

    // and back to zero
    voicemailCountSpy.clear();
    mMockController->setVoicemailCount(0);
    QTRY_COMPARE(voicemailCountSpy.count(), 1);
    QCOMPARE((int)mAccount->voicemailCount(), 0);
}

void OfonoAccountEntryTest::testSimLocked()
{
    QSignalSpy simLockedSpy(mAccount, SIGNAL(simLockedChanged()));

    // check that it is not locked by default
    QVERIFY(!mAccount->simLocked());

    // now try to set the status to simlocked
    mMockController->setPresence("simlocked", "simlocked");
    QTRY_COMPARE(simLockedSpy.count(), 1);
    QVERIFY(mAccount->simLocked());
}

void OfonoAccountEntryTest::testEmergencyCallsAvailable_data()
{
    QTest::addColumn<QString>("status");
    QTest::addColumn<bool>("available");

    QTest::newRow("available") << "available" << true;
    QTest::newRow("away") << "away" << true;
    QTest::newRow("simlocked") << "simlocked" << true;
    QTest::newRow("flightmode") << "flightmode" << false;
    QTest::newRow("nosim") << "nosim" << true;
    QTest::newRow("nomodem") << "nomodem" << false;
    QTest::newRow("registered") << "registered" << true;
    QTest::newRow("roaming") << "roaming" << true;
    QTest::newRow("unregistered") << "unregistered" << true;
    QTest::newRow("denied") << "denied" << true;
    QTest::newRow("unknown") << "unknown" << true;
    QTest::newRow("searching") << "searching" << true;
}

void OfonoAccountEntryTest::testEmergencyCallsAvailable()
{
    QFETCH(QString, status);
    QFETCH(bool, available);

    QSignalSpy statusChangedSpy(mAccount, SIGNAL(statusChanged()));
    mMockController->setPresence(status, "");

    QTRY_COMPARE(statusChangedSpy.count(), 1);
    QCOMPARE(mAccount->status(), status);
    QCOMPARE(mAccount->emergencyCallsAvailable(), available);
}

QTEST_MAIN(OfonoAccountEntryTest)
#include "OfonoAccountEntryTest.moc"
