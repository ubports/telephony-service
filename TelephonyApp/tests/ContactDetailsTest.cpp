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

private:
    QString currentTestValue();
    QString newTestValue();
    void resetTestValue();

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
    int mCount;
};

void ContactDetailsTest::resetTestValue()
{
    mCount = 0;
}

QString ContactDetailsTest::newTestValue()
{
    mTestValue = QString("testValue%1").arg(++mCount);
    return mTestValue;
}

QString ContactDetailsTest::currentTestValue()
{
    return mTestValue;
}

void ContactDetailsTest::initTestCase()
{
    mCount = 0;
}

void ContactDetailsTest::testContactAddress()
{
    ContactAddress address;

    QSignalSpy signalSpy(&address, SIGNAL(changed()));

    QSignalSpy countryChangedSignalSpy(&address, SIGNAL(countryChanged()));
    address.setCountry(newTestValue());
    QCOMPARE(address.country(), currentTestValue());
    QCOMPARE(countryChangedSignalSpy.count(), 1);

    QSignalSpy localityChangedSignalSpy(&address, SIGNAL(localityChanged()));
    address.setLocality(newTestValue());
    QCOMPARE(address.locality(), currentTestValue());
    QCOMPARE(localityChangedSignalSpy.count(), 1);

    QSignalSpy postOfficeBoxChangedSignalSpy(&address, SIGNAL(postOfficeBoxChanged()));
    address.setPostOfficeBox(newTestValue());
    QCOMPARE(address.postOfficeBox(), currentTestValue());
    QCOMPARE(postOfficeBoxChangedSignalSpy.count(), 1);

    QSignalSpy postcodeChangedSignalSpy(&address, SIGNAL(postcodeChanged()));
    address.setPostcode(newTestValue());
    QCOMPARE(address.postcode(), currentTestValue());
    QCOMPARE(postcodeChangedSignalSpy.count(), 1);

    QSignalSpy regionChangedSignalSpy(&address, SIGNAL(regionChanged()));
    address.setRegion(newTestValue());
    QCOMPARE(address.region(), currentTestValue());
    QCOMPARE(regionChangedSignalSpy.count(), 1);

    QSignalSpy streetChangedSignalSpy(&address, SIGNAL(streetChanged()));
    address.setStreet(newTestValue());
    QCOMPARE(address.street(), currentTestValue());
    QCOMPARE(streetChangedSignalSpy.count(), 1);

    QSignalSpy subTypesChangedSignalSpy(&address, SIGNAL(subTypesChanged()));
    address.setSubTypes(newTestValue());
    QCOMPARE(address.subTypes(), QVariant(currentTestValue()));
    QCOMPARE(subTypesChangedSignalSpy.count(), 1);

    QCOMPARE(signalSpy.count(), 7);

    resetTestValue();
    QContactAddress qcontactaddress;
    qcontactaddress.setCountry(newTestValue());
    qcontactaddress.setLocality(newTestValue());
    qcontactaddress.setPostOfficeBox(newTestValue());
    qcontactaddress.setPostcode(newTestValue());
    qcontactaddress.setRegion(newTestValue());
    qcontactaddress.setStreet(newTestValue());
    qcontactaddress.setSubTypes(newTestValue());

    resetTestValue();
    ContactAddress address2(qcontactaddress);
    QCOMPARE(address2.country(), newTestValue());
    QCOMPARE(address2.locality(), newTestValue());
    QCOMPARE(address2.postOfficeBox(), newTestValue());
    QCOMPARE(address2.postcode(), newTestValue());
    QCOMPARE(address2.region(), newTestValue());
    QCOMPARE(address2.street(), newTestValue());
    QCOMPARE(address2.subTypes(), QVariant(newTestValue()));
}

void ContactDetailsTest::testContactCustomId()
{
    ContactCustomId customId;

    customId.setCustomId(newTestValue());
    QCOMPARE(customId.customId(), currentTestValue());
}

void ContactDetailsTest::testContactEmailAddress()
{
    ContactEmailAddress email;

    QSignalSpy signalSpy(&email, SIGNAL(changed()));

    email.setEmailAddress(newTestValue());
    QCOMPARE(email.emailAddress(), currentTestValue());

    QCOMPARE(signalSpy.count(), 1);

    QContactEmailAddress qcontactemailaddress;
    qcontactemailaddress.setEmailAddress(newTestValue());

    ContactEmailAddress email2(qcontactemailaddress);
    QCOMPARE(email2.emailAddress(), currentTestValue());
}

void ContactDetailsTest::testContactName()
{
    ContactName name;

    QSignalSpy signalSpy(&name, SIGNAL(changed()));

    QSignalSpy customLabelChangedSignalSpy(&name, SIGNAL(customLabelChanged()));
    name.setCustomLabel(newTestValue());
    QCOMPARE(name.customLabel(), currentTestValue());
    QCOMPARE(customLabelChangedSignalSpy.count(), 1);

    QSignalSpy firstNameChangedSignalSpy(&name, SIGNAL(firstNameChanged()));
    name.setFirstName(newTestValue());
    QCOMPARE(name.firstName(), currentTestValue());
    QCOMPARE(firstNameChangedSignalSpy.count(), 1);

    QSignalSpy lastNameChangedSignalSpy(&name, SIGNAL(lastNameChanged()));
    name.setLastName(newTestValue());
    QCOMPARE(name.lastName(), currentTestValue());
    QCOMPARE(lastNameChangedSignalSpy.count(), 1);

    QSignalSpy middleNameChangedSignalSpy(&name, SIGNAL(middleNameChanged()));
    name.setMiddleName(newTestValue());
    QCOMPARE(name.middleName(), currentTestValue());
    QCOMPARE(middleNameChangedSignalSpy.count(), 1);

    QSignalSpy prefixChangedSignalSpy(&name, SIGNAL(prefixChanged()));
    name.setPrefix(newTestValue());
    QCOMPARE(name.prefix(), currentTestValue());
    QCOMPARE(prefixChangedSignalSpy.count(), 1);

    QSignalSpy suffixChangedSignalSpy(&name, SIGNAL(suffixChanged()));
    name.setSuffix(newTestValue());
    QCOMPARE(name.suffix(), currentTestValue());
    QCOMPARE(suffixChangedSignalSpy.count(), 1);

    QCOMPARE(signalSpy.count(), 6);

    resetTestValue();
    QContactName qcontactname;
    qcontactname.setCustomLabel(newTestValue());
    qcontactname.setFirstName(newTestValue());
    qcontactname.setLastName(newTestValue());
    qcontactname.setMiddleName(newTestValue());
    qcontactname.setPrefix(newTestValue());
    qcontactname.setSuffix(newTestValue());

    resetTestValue();
    ContactName name2(qcontactname);
    QCOMPARE(name2.customLabel(), newTestValue());
    QCOMPARE(name2.firstName(), newTestValue());
    QCOMPARE(name2.lastName(), newTestValue());
    QCOMPARE(name2.middleName(), newTestValue());
    QCOMPARE(name2.prefix(), newTestValue());
    QCOMPARE(name2.suffix(), newTestValue());
}

void ContactDetailsTest::testContactOnlineAccount()
{
    ContactOnlineAccount onlineAccount;

    QSignalSpy signalSpy(&onlineAccount, SIGNAL(changed()));

    QSignalSpy accountUriChangedSignalSpy(&onlineAccount, SIGNAL(accountUriChanged()));
    onlineAccount.setAccountUri(newTestValue());
    QCOMPARE(onlineAccount.accountUri(), currentTestValue());
    QCOMPARE(accountUriChangedSignalSpy.count(), 1);

    QSignalSpy capabilitiesChangedSignalSpy(&onlineAccount, SIGNAL(capabilitiesChanged()));
    onlineAccount.setCapabilities(newTestValue());
    QCOMPARE(onlineAccount.capabilities(), QVariant(currentTestValue()));
    QCOMPARE(capabilitiesChangedSignalSpy.count(), 1);

    QSignalSpy protocolChangedSignalSpy(&onlineAccount, SIGNAL(protocolChanged()));
    onlineAccount.setProtocol(newTestValue());
    QCOMPARE(onlineAccount.protocol(), currentTestValue());
    QCOMPARE(protocolChangedSignalSpy.count(), 1);

    QSignalSpy serviceProviderChangedSignalSpy(&onlineAccount, SIGNAL(serviceProviderChanged()));
    onlineAccount.setServiceProvider(newTestValue());
    QCOMPARE(onlineAccount.serviceProvider(), currentTestValue());
    QCOMPARE(serviceProviderChangedSignalSpy.count(), 1);

    QSignalSpy subTypesChangedSignalSpy(&onlineAccount, SIGNAL(subTypesChanged()));
    onlineAccount.setSubTypes(newTestValue());
    QCOMPARE(onlineAccount.subTypes(), QVariant(currentTestValue()));
    QCOMPARE(subTypesChangedSignalSpy.count(), 1);

    QCOMPARE(signalSpy.count(), 5);

    resetTestValue();
    QContactOnlineAccount qcontactonlineaccount;
    qcontactonlineaccount.setAccountUri(newTestValue());
    qcontactonlineaccount.setCapabilities(QStringList(newTestValue()));
    qcontactonlineaccount.setProtocol(newTestValue());
    qcontactonlineaccount.setServiceProvider(newTestValue());

    resetTestValue();
    ContactOnlineAccount onlineAccount2(qcontactonlineaccount);
    QCOMPARE(onlineAccount2.accountUri(), newTestValue());
    QCOMPARE(onlineAccount2.capabilities(), QVariant(newTestValue()));
    QCOMPARE(onlineAccount2.protocol(), newTestValue());
    QCOMPARE(onlineAccount2.serviceProvider(), newTestValue());
}

void ContactDetailsTest::testContactPhoneNumber()
{
    ContactPhoneNumber phoneNumber;

    QSignalSpy signalSpy(&phoneNumber, SIGNAL(changed()));

    QSignalSpy numberChangedSignalSpy(&phoneNumber, SIGNAL(numberChanged()));
    phoneNumber.setNumber(newTestValue());
    QCOMPARE(phoneNumber.number(), currentTestValue());
    QCOMPARE(numberChangedSignalSpy.count(), 1);

    QSignalSpy subTypesChangedSignalSpy(&phoneNumber, SIGNAL(subTypesChanged()));
    phoneNumber.setSubTypes(newTestValue());
    QCOMPARE(phoneNumber.subTypes(), QVariant(currentTestValue()));
    QCOMPARE(subTypesChangedSignalSpy.count(), 1);

    QCOMPARE(signalSpy.count(), 2);

    resetTestValue();
    QContactPhoneNumber qcontactphonenumber;
    qcontactphonenumber.setNumber(newTestValue());
    qcontactphonenumber.setSubTypes(newTestValue());

    resetTestValue();
    ContactPhoneNumber phoneNumber2(qcontactphonenumber);
    QCOMPARE(phoneNumber2.number(), newTestValue());
    QCOMPARE(phoneNumber2.subTypes(), QVariant(newTestValue()));
}

QTEST_MAIN(ContactDetailsTest)
#include "ContactDetailsTest.moc"
