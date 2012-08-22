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
#include <QContact>
#include <QContactAddress>
#include <QContactAvatar>
#include <QContactDetailDefinition>
#include <QContactEmailAddress>
#include <QContactGuid>
#include <QContactName>
#include <QContactOnlineAccount>
#include <QContactPhoneNumber>
#include "contactaddress.h"
#include "contactcustomid.h"
#include "contactemailaddress.h"
#include "contactentry.h"
#include "contactmodel.h"
#include "contactname.h"
#include "contactonlineaccount.h"
#include "contactphonenumber.h"

using namespace QtMobility;

class ContactEntryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testConstructor();
    void testId();
    void testCustomId();
    void testDisplayLabel();
    void testInitial_data();
    void testInitial();
    void testAvatar();
    void testName();
    void testModified();
    void testAddresses();
    void testEmails();
    void testOnlineAccounts();
    void testPhoneNumbers();

private:
    ContactModel *contactModel;
};

void ContactEntryTest::initTestCase()
{
    contactModel = ContactModel::instance("memory");
}

void ContactEntryTest::testConstructor()
{
    QContact contact;
    ContactEntry entry(contact);
    QCOMPARE(entry.contact(), contact);
}

void ContactEntryTest::testId()
{
    QContact contact;
    QContactGuid guidDetail;
    guidDetail.setGuid("testcontactid");
    QVERIFY(contact.saveDetail(&guidDetail));
    ContactEntry entry(contact);
    QCOMPARE(entry.id(), guidDetail.guid());
}

void ContactEntryTest::testCustomId()
{
    QContact contact;
    ContactCustomId customIdDetail;
    QString customId("testcustomid");
    customIdDetail.setCustomId(QString("anotherid:%1").arg(customId));
    QVERIFY(contact.saveDetail(&customIdDetail));
    ContactEntry entry(contact);
    QCOMPARE(entry.customId(), customId);
}

void ContactEntryTest::testDisplayLabel()
{
    QContact contact;
    QContactName nameDetail;
    nameDetail.setCustomLabel("Test Display Label");
    QVERIFY(contact.saveDetail(&nameDetail));
    ContactEntry entry(contact);
    QCOMPARE(entry.displayLabel(), nameDetail.customLabel());
}

void ContactEntryTest::testInitial_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("initial");

    QTest::newRow("name with capital letter") << "Test Contact" << "T";
    QTest::newRow("all lowercase name") << "test contact" << "T";
    QTest::newRow("accentuated letter") << QString::fromUtf8("Ánother test") << QString::fromUtf8("Á");
    QTest::newRow("accentuated lowercase name") << QString::fromUtf8("éndless test") << QString::fromUtf8("É");
}

void ContactEntryTest::testInitial()
{
    QFETCH(QString, name);
    QFETCH(QString, initial);

    QContact contact;
    QContactName nameDetail;
    nameDetail.setCustomLabel(name);
    QVERIFY(contact.saveDetail(&nameDetail));
    ContactEntry entry(contact);
    QCOMPARE(entry.initial(), initial);
}

void ContactEntryTest::testAvatar()
{
    QContact contact;
    QContactAvatar avatarDetail;
    avatarDetail.setImageUrl(QUrl::fromLocalFile("/path/to/some/image.png"));
    QVERIFY(contact.saveDetail(&avatarDetail));
    ContactEntry entry(contact);
    QCOMPARE(entry.avatar(), avatarDetail.imageUrl());
}

void ContactEntryTest::testName()
{
    // check if the name detail is created for a contact that doesn't have a name detail
    ContactEntry emptyEntry;
    ContactName *emptyName = emptyEntry.name();
    QVERIFY(emptyName);

    // and test if the name detail is property loaded for contacts that have it
    QContact contact;
    QContactName nameDetail;
    nameDetail.setFirstName("First");
    nameDetail.setLastName("Last");
    nameDetail.setCustomLabel("Custom Label");
    QVERIFY(contact.saveDetail(&nameDetail));
    ContactEntry entry(contact);
    ContactName *name = entry.name();
    QVERIFY(name);
    QCOMPARE(name->firstName(), nameDetail.firstName());
    QCOMPARE(name->lastName(), nameDetail.lastName());
    QCOMPARE(name->customLabel(), nameDetail.customLabel());
}

void ContactEntryTest::testModified()
{
    ContactEntry entry;
    QSignalSpy signalSpy(&entry, SIGNAL(modifiedChanged()));

    QCOMPARE(entry.modified(), false);
    entry.setModified(true);
    QCOMPARE(signalSpy.count(), 1);
    QCOMPARE(entry.modified(), true);
    entry.setModified(false);
    QCOMPARE(signalSpy.count(), 2);
    QCOMPARE(entry.modified(), false);
}

void ContactEntryTest::testAddresses()
{
    QContact contact;
    for (int i=0; i < 10; ++i) {
        QContactAddress addressDetail;
        addressDetail.setStreet(QString("Street %1").arg(i));
        QVERIFY(contact.saveDetail(&addressDetail));
    }

    ContactEntry entry(contact);
    QDeclarativeListProperty<ContactDetail> addresses = entry.addresses();
    QCOMPARE(ContactEntry::detailCount(&addresses), 10);

    ContactAddress address;
    address.setStreet("Street 10");
    ContactEntry::detailAppend(&addresses, &address);
    QCOMPARE(ContactEntry::detailCount(&addresses), 11);

    for (int i=0; i < 11; ++i) {
        ContactAddress *address = qobject_cast<ContactAddress*>(ContactEntry::detailAt(&addresses, i));
        QVERIFY(address);
        QCOMPARE(address->street(), QString("Street %1").arg(i));
    }
}

void ContactEntryTest::testEmails()
{
    QContact contact;
    for (int i=0; i < 10; ++i) {
        QContactEmailAddress emailDetail;
        emailDetail.setEmailAddress(QString("contact%1@foo.bar").arg(i));
        QVERIFY(contact.saveDetail(&emailDetail));
    }

    ContactEntry entry(contact);
    QDeclarativeListProperty<ContactDetail> emails = entry.emails();
    QCOMPARE(ContactEntry::detailCount(&emails), 10);

    ContactEmailAddress email;
    email.setEmailAddress("contact10@foo.bar");
    ContactEntry::detailAppend(&emails, &email);
    QCOMPARE(ContactEntry::detailCount(&emails), 11);

    for (int i=0; i < 11; ++i) {
        ContactEmailAddress *email = qobject_cast<ContactEmailAddress*>(ContactEntry::detailAt(&emails, i));
        QVERIFY(email);
        QCOMPARE(email->emailAddress(), QString("contact%1@foo.bar").arg(i));
    }
}

void ContactEntryTest::testOnlineAccounts()
{
    QContact contact;
    for (int i=0; i < 10; ++i) {
        QContactOnlineAccount onlineAccountDetail;
        onlineAccountDetail.setAccountUri(QString("contact%1@foo.bar").arg(i));
        QVERIFY(contact.saveDetail(&onlineAccountDetail));
    }

    ContactEntry entry(contact);
    QDeclarativeListProperty<ContactDetail> onlineAccounts = entry.onlineAccounts();
    QCOMPARE(ContactEntry::detailCount(&onlineAccounts), 10);

    ContactOnlineAccount onlineAccount;
    onlineAccount.setAccountUri("contact10@foo.bar");
    ContactEntry::detailAppend(&onlineAccounts, &onlineAccount);
    QCOMPARE(ContactEntry::detailCount(&onlineAccounts), 11);

    for (int i=0; i < 11; ++i) {
        ContactOnlineAccount *onlineAccount = qobject_cast<ContactOnlineAccount*>(ContactEntry::detailAt(&onlineAccounts, i));
        QVERIFY(onlineAccount);
        QCOMPARE(onlineAccount->accountUri(), QString("contact%1@foo.bar").arg(i));
    }
}

void ContactEntryTest::testPhoneNumbers()
{
    QContact contact;
    for (int i=0; i < 10; ++i) {
        QContactPhoneNumber phoneNumberDetail;
        phoneNumberDetail.setNumber(QString("12345-%1").arg(i));
        QVERIFY(contact.saveDetail(&phoneNumberDetail));
    }

    ContactEntry entry(contact);
    QDeclarativeListProperty<ContactDetail> phoneNumbers = entry.phoneNumbers();
    QCOMPARE(ContactEntry::detailCount(&phoneNumbers), 10);

    ContactPhoneNumber phoneNumber;
    phoneNumber.setNumber("12345-10");
    ContactEntry::detailAppend(&phoneNumbers, &phoneNumber);
    QCOMPARE(ContactEntry::detailCount(&phoneNumbers), 11);

    for (int i=0; i < 11; ++i) {
        ContactPhoneNumber *phoneNumber = qobject_cast<ContactPhoneNumber*>(ContactEntry::detailAt(&phoneNumbers, i));
        QVERIFY(phoneNumber);
        QCOMPARE(phoneNumber->number(), QString("12345-%1").arg(i));
    }
}

QTEST_MAIN(ContactEntryTest)
#include "ContactEntryTest.moc"
