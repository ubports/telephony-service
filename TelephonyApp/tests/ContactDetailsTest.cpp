/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include "contactaddress.h"
#include "contactcustomid.cpp"
#include "contactemailaddress.cpp"
#include "contactname.cpp"
#include "contactonlineaccount.cpp"
#include "contactphonenumber.cpp"

using namespace QtMobility;

class ContactDetailsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testContactAddress();
    void testContactCustomId();
    void testContactEmailAddress();
    void testContactName();
    void testContactOnlineAccount();
    void testContactPhoneNumber();
private:
    QString mTestValue;
};

void ContactDetailsTest::initTestCase()
{
    mTestValue = QString("testValue");
}

void ContactDetailsTest::testContactAddress()
{
    ContactAddress address;

    QSignalSpy signalSpy(&address, SIGNAL(changed()));

    address.setCountry(mTestValue);
    QCOMPARE(address.country(), mTestValue);

    address.setLocality(mTestValue);
    QCOMPARE(address.locality(), mTestValue);

    address.setPostOfficeBox(mTestValue);
    QCOMPARE(address.postOfficeBox(), mTestValue);

    address.setPostcode(mTestValue);
    QCOMPARE(address.postcode(), mTestValue);

    address.setRegion(mTestValue);
    QCOMPARE(address.region(), mTestValue);

    address.setStreet(mTestValue);
    QCOMPARE(address.street(), mTestValue);

    address.setSubTypes(mTestValue);
    QCOMPARE(address.subTypes(), QVariant(mTestValue));

    QCOMPARE(signalSpy.count(), 7);

    QContactAddress qcontactaddress;
    qcontactaddress.setCountry(mTestValue);
    qcontactaddress.setLocality(mTestValue);
    qcontactaddress.setPostOfficeBox(mTestValue);
    qcontactaddress.setPostcode(mTestValue);
    qcontactaddress.setRegion(mTestValue);
    qcontactaddress.setStreet(mTestValue);
    qcontactaddress.setSubTypes(mTestValue);

    ContactAddress address2(qcontactaddress);
    QCOMPARE(address2.country(), mTestValue);
    QCOMPARE(address2.locality(), mTestValue);
    QCOMPARE(address2.postOfficeBox(), mTestValue);
    QCOMPARE(address2.postcode(), mTestValue);
    QCOMPARE(address2.region(), mTestValue);
    QCOMPARE(address2.street(), mTestValue);
    QCOMPARE(address2.subTypes(), QVariant(mTestValue));

}

void ContactDetailsTest::testContactCustomId()
{
    ContactCustomId customId;

    customId.setCustomId(mTestValue);
    QCOMPARE(customId.customId(), mTestValue);
}

void ContactDetailsTest::testContactEmailAddress()
{
    ContactEmailAddress email;

    QSignalSpy signalSpy(&email, SIGNAL(changed()));

    email.setEmailAddress(mTestValue);
    QCOMPARE(email.emailAddress(), mTestValue);

    QCOMPARE(signalSpy.count(), 1);

    QContactEmailAddress qcontactemailaddress;
    qcontactemailaddress.setEmailAddress(mTestValue);

    ContactEmailAddress email2(qcontactemailaddress);
    QCOMPARE(email2.emailAddress(), mTestValue);
}

void ContactDetailsTest::testContactName()
{
    ContactName name;

    QSignalSpy signalSpy(&name, SIGNAL(changed()));

    name.setCustomLabel(mTestValue);
    QCOMPARE(name.customLabel(), mTestValue);

    name.setFirstName(mTestValue);
    QCOMPARE(name.firstName(), mTestValue);

    name.setLastName(mTestValue);
    QCOMPARE(name.lastName(), mTestValue);

    name.setMiddleName(mTestValue);
    QCOMPARE(name.middleName(), mTestValue);

    name.setPrefix(mTestValue);
    QCOMPARE(name.prefix(), mTestValue);

    name.setSuffix(mTestValue);
    QCOMPARE(name.suffix(), mTestValue);

    QCOMPARE(signalSpy.count(), 6);

    QContactName qcontactname;

    qcontactname.setCustomLabel(mTestValue);
    qcontactname.setFirstName(mTestValue);
    qcontactname.setLastName(mTestValue);
    qcontactname.setMiddleName(mTestValue);
    qcontactname.setPrefix(mTestValue);
    qcontactname.setSuffix(mTestValue);

    ContactName name2(qcontactname);

    QCOMPARE(name2.customLabel(), mTestValue);
    QCOMPARE(name2.firstName(), mTestValue);
    QCOMPARE(name2.lastName(), mTestValue);
    QCOMPARE(name2.middleName(), mTestValue);
    QCOMPARE(name2.prefix(), mTestValue);
    QCOMPARE(name2.suffix(), mTestValue);
}

void ContactDetailsTest::testContactOnlineAccount()
{
    ContactOnlineAccount onlineAccount;

    QSignalSpy signalSpy(&onlineAccount, SIGNAL(changed()));

    onlineAccount.setAccountUri(mTestValue);
    QCOMPARE(onlineAccount.accountUri(), mTestValue);

    onlineAccount.setCapabilities(mTestValue);
    QCOMPARE(onlineAccount.capabilities(), QVariant(mTestValue));

    onlineAccount.setProtocol(mTestValue);
    QCOMPARE(onlineAccount.protocol(), mTestValue);

    onlineAccount.setServiceProvider(mTestValue);
    QCOMPARE(onlineAccount.serviceProvider(), mTestValue);

    QCOMPARE(signalSpy.count(), 4);

    QContactOnlineAccount qcontactonlineaccount;

    qcontactonlineaccount.setAccountUri(mTestValue);
    qcontactonlineaccount.setCapabilities(QStringList(mTestValue));
    qcontactonlineaccount.setProtocol(mTestValue);
    qcontactonlineaccount.setServiceProvider(mTestValue);

    ContactOnlineAccount onlineAccount2(qcontactonlineaccount);
    QCOMPARE(onlineAccount2.accountUri(), mTestValue);
    QCOMPARE(onlineAccount2.capabilities(), QVariant(mTestValue));
    QCOMPARE(onlineAccount2.protocol(), mTestValue);
    QCOMPARE(onlineAccount2.serviceProvider(), mTestValue);
}

void ContactDetailsTest::testContactPhoneNumber()
{
    ContactPhoneNumber phoneNumber;

    QSignalSpy signalSpy(&phoneNumber, SIGNAL(changed()));

    phoneNumber.setNumber(mTestValue);
    QCOMPARE(phoneNumber.number(), mTestValue);

    phoneNumber.setSubTypes(mTestValue);
    QCOMPARE(phoneNumber.subTypes(), QVariant(mTestValue));

    QCOMPARE(signalSpy.count(), 2);

    QContactPhoneNumber qcontactphonenumber;
    qcontactphonenumber.setNumber(mTestValue);
    qcontactphonenumber.setSubTypes(mTestValue);

    ContactPhoneNumber phoneNumber2(qcontactphonenumber);

    QCOMPARE(phoneNumber2.number(), mTestValue);
    QCOMPARE(phoneNumber2.subTypes(), QVariant(mTestValue));
}

QTEST_MAIN(ContactDetailsTest)
#include "ContactDetailsTest.moc"
