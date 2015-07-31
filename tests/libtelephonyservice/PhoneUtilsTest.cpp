/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#include "phoneutils.h"

Q_DECLARE_METATYPE(PhoneUtils::PhoneNumberMatchType)

class PhoneUtilsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testIsPhoneNumber_data();
    void testIsPhoneNumber();
    void testComparePhoneNumbers_data();
    void testComparePhoneNumbers();
};

void PhoneUtilsTest::testIsPhoneNumber_data()
{
    QTest::addColumn<QString>("number");
    QTest::addColumn<bool>("expectedResult");

    QTest::newRow("simple number") << "12345678" << true;
    QTest::newRow("number with dash") << "1234-5678" << true;
    QTest::newRow("number with area code") << "(123)12345678" << true;
    QTest::newRow("number with extension") << "12345678#123" << true;
    QTest::newRow("number with slash") << "+421 2/123 456 78" << true;
    QTest::newRow("short/emergency number") << "190" << true;
    QTest::newRow("non phone numbers") << "abcdefg" << false;
}

void PhoneUtilsTest::testIsPhoneNumber()
{
    QFETCH(QString, number);
    QFETCH(bool, expectedResult);

    bool result = PhoneUtils::isPhoneNumber(number);
    QCOMPARE(result, expectedResult);
}

void PhoneUtilsTest::testComparePhoneNumbers_data()
{
    QTest::addColumn<QString>("number1");
    QTest::addColumn<QString>("number2");
    QTest::addColumn<PhoneUtils::PhoneNumberMatchType>("expectedResult");

    QTest::newRow("string equal") << "12345678" << "12345678" << PhoneUtils::NSN_MATCH;
    QTest::newRow("number with dash") << "1234-5678" << "12345678" << PhoneUtils::NSN_MATCH;
    QTest::newRow("number with area code") << "1231234567" << "1234567" << PhoneUtils::SHORT_NSN_MATCH;
    QTest::newRow("number with extension") << "12345678#123" << "12345678" << PhoneUtils::SHORT_NSN_MATCH;
    QTest::newRow("both numbers with extension") << "(123)12345678#1" << "12345678#1" << PhoneUtils::SHORT_NSN_MATCH;
    QTest::newRow("numbers with different extension") << "1234567#1" << "1234567#2" << PhoneUtils::NO_MATCH;
    QTest::newRow("short/emergency numbers") << "190" << "190" << PhoneUtils::EXACT_MATCH;
    QTest::newRow("different short/emergency numbers") << "911" << "11" << PhoneUtils::NO_MATCH;
    QTest::newRow("different numbers") << "12345678" << "1234567" << PhoneUtils::NO_MATCH;
    QTest::newRow("both non phone numbers") << "abcdefg" << "abcdefg" << PhoneUtils::EXACT_MATCH;
    QTest::newRow("different non phone numbers") << "abcdefg" << "bcdefg" << PhoneUtils::INVALID_NUMBER;
    QTest::newRow("phone number and custom string") << "abc12345678" << "12345678" << PhoneUtils::NSN_MATCH;
    // FIXME: check what other cases we need to test here"
}

void PhoneUtilsTest::testComparePhoneNumbers()
{
    QFETCH(QString, number1);
    QFETCH(QString, number2);
    QFETCH(PhoneUtils::PhoneNumberMatchType, expectedResult);

    PhoneUtils::PhoneNumberMatchType result = PhoneUtils::comparePhoneNumbers(number1, number2);
    QCOMPARE(result, expectedResult);
}

QTEST_MAIN(PhoneUtilsTest)
#include "PhoneUtilsTest.moc"
