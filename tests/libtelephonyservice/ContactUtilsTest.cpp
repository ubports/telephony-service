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

#include "contactutils.h"
#include <QContactName>

QTCONTACTS_USE_NAMESPACE

class ContactUtilsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testFormatContactName_data();
    void testFormatContactName();
    void testSharedManagerCreation();
};

void ContactUtilsTest::testFormatContactName_data()
{
    QTest::addColumn<QString>("firstName");
    QTest::addColumn<QString>("lastName");
    QTest::addColumn<QString>("expectedResult");

    QTest::newRow("full name") << "First" << "Last" << "First Last";
    QTest::newRow("only first name") << "First" << "" << "First";
    QTest::newRow("only last name") << "" << "Last" << "Last";
}

void ContactUtilsTest::testFormatContactName()
{
    QFETCH(QString, firstName);
    QFETCH(QString, lastName);
    QFETCH(QString, expectedResult);

    // create the contact
    QContact contact;
    QContactName contactName;
    contactName.setFirstName(firstName);
    contactName.setLastName(lastName);
    contact.saveDetail(&contactName);

    QString result = ContactUtils::formatContactName(contact);
    QCOMPARE(result, expectedResult);
}

void ContactUtilsTest::testSharedManagerCreation()
{
    QContactManager *manager = ContactUtils::sharedManager("memory");
    QVERIFY(manager);
    QCOMPARE(manager->managerName(), QString("memory"));
}

QTEST_MAIN(ContactUtilsTest)
#include "ContactUtilsTest.moc"
