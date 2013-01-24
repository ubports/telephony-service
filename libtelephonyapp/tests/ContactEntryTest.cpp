/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
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
#include <QContactEmailAddress>
#include <QContactName>
#include <QContactOnlineAccount>
#include <QContactPhoneNumber>
#include "contactaddress.h"
#include "contactemailaddress.h"
#include "contactentry.h"
#include "contactmodel.h"
#include "contactname.h"
#include "contactonlineaccount.h"
#include "contactphonenumber.h"

QTCONTACTS_USE_NAMESPACE

class ContactEntryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testConstructor();
    void testId();
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
    void testContact();
    void testAddDetail();
    void testRemoveDetail();
    void testRevertChanges();

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
    // FIXME: check how to properly test contact ids
#if 0
    QContact contact;
    QContactGuid guidDetail;
    guidDetail.setGuid("testcontactid");
    QVERIFY(contact.saveDetail(&guidDetail));
    ContactEntry entry(contact);
    QCOMPARE(entry.id(), guidDetail.guid());
#endif
}

void ContactEntryTest::testDisplayLabel()
{
    QContact contact;
    QContactDisplayLabel labelDetail;
    labelDetail.setLabel("Test Display Label");
    QVERIFY(contact.saveDetail(&labelDetail));
    ContactEntry entry(contact);
    QCOMPARE(entry.displayLabel(), labelDetail.label());

    QSignalSpy signalSpy(&entry, SIGNAL(changed(ContactEntry*)));
    QString testLabel("Another Display Label");
    entry.setDisplayLabel(testLabel);
    QCOMPARE(signalSpy.count(), 1);
    QCOMPARE(entry.displayLabel(), testLabel);
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
    QContactDisplayLabel labelDetail;
    labelDetail.setLabel(name);
    QVERIFY(contact.saveDetail(&labelDetail));
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
    QVERIFY(contact.saveDetail(&nameDetail));
    ContactEntry entry(contact);
    ContactName *name = entry.name();
    QVERIFY(name);
    QCOMPARE(name->firstName(), nameDetail.firstName());
    QCOMPARE(name->lastName(), nameDetail.lastName());
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
    QQmlListProperty<ContactDetail> addresses = entry.addresses();
    QCOMPARE(ContactEntry::detailCount(&addresses), 10);

    for (int i=0; i < 10; ++i) {
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
    QQmlListProperty<ContactDetail> emails = entry.emails();
    QCOMPARE(ContactEntry::detailCount(&emails), 10);

    for (int i=0; i < 10; ++i) {
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
    QQmlListProperty<ContactDetail> onlineAccounts = entry.onlineAccounts();
    QCOMPARE(ContactEntry::detailCount(&onlineAccounts), 10);

    for (int i=0; i < 10; ++i) {
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
    QQmlListProperty<ContactDetail> phoneNumbers = entry.phoneNumbers();
    QCOMPARE(ContactEntry::detailCount(&phoneNumbers), 10);

    for (int i=0; i < 10; ++i) {
        ContactPhoneNumber *phoneNumber = qobject_cast<ContactPhoneNumber*>(ContactEntry::detailAt(&phoneNumbers, i));
        QVERIFY(phoneNumber);
        QCOMPARE(phoneNumber->number(), QString("12345-%1").arg(i));
    }
}

void ContactEntryTest::testContact()
{
    QContact contact;
    ContactEntry entry(contact);
    QCOMPARE(entry.contact(), contact);
}

void ContactEntryTest::testAddDetail()
{
    // test one detail of each type
    ContactEntry entry;
    ContactPhoneNumber phoneNumber;
    ContactAddress address;
    ContactOnlineAccount onlineAccount;
    QSignalSpy signalSpy(&entry, SIGNAL(modifiedChanged()));

    QVERIFY(entry.addDetail(&phoneNumber));
    QCOMPARE(signalSpy.count(), 1);
    QVERIFY(entry.modified());
    QQmlListProperty<ContactDetail> phoneNumbers = entry.phoneNumbers();
    QCOMPARE(entry.detailCount(&phoneNumbers), 1);
    QCOMPARE(entry.detailAt(&phoneNumbers, 0)->detail(), phoneNumber.detail());

    QVERIFY(entry.addDetail(&address));
    QQmlListProperty<ContactDetail> addresses = entry.addresses();
    QCOMPARE(entry.detailCount(&addresses), 1);
    QCOMPARE(entry.detailAt(&addresses, 0)->detail(), address.detail());

    QVERIFY(entry.addDetail(&onlineAccount));
    QQmlListProperty<ContactDetail> onlineAccounts = entry.onlineAccounts();
    QCOMPARE(entry.detailCount(&onlineAccounts), 1);
    QCOMPARE(entry.detailAt(&onlineAccounts, 0)->detail(), onlineAccount.detail());
}

void ContactEntryTest::testRemoveDetail()
{
    // test one detail of each type
    // the details are only removed from the QContact object, but they are kept in the
    // ContactEntry detail list because QML items can still use them to animate the removal.
    QContact contact;
    QContactPhoneNumber phoneNumber;
    QContactAddress address;
    QContactOnlineAccount onlineAccount;
    QVERIFY(contact.saveDetail(&phoneNumber));
    QVERIFY(contact.saveDetail(&address));
    QVERIFY(contact.saveDetail(&onlineAccount));

    ContactEntry entry(contact);
    QSignalSpy signalSpy(&entry, SIGNAL(modifiedChanged()));

    QQmlListProperty<ContactDetail> phoneNumbers = entry.phoneNumbers();
    QVERIFY(entry.removeDetail(entry.detailAt(&phoneNumbers, 0)));
    QCOMPARE(signalSpy.count(), 1);
    QVERIFY(entry.modified());
    QCOMPARE(entry.detailCount(&phoneNumbers), 1);
    QCOMPARE(entry.contact().details<QContactPhoneNumber>().count(), 0);

    QQmlListProperty<ContactDetail> addresses = entry.addresses();
    QVERIFY(entry.removeDetail(entry.detailAt(&addresses, 0)));
    QCOMPARE(entry.detailCount(&addresses), 1);
    QCOMPARE(entry.contact().details<QContactAddress>().count(), 0);

    QQmlListProperty<ContactDetail> onlineAccounts = entry.onlineAccounts();
    QVERIFY(entry.removeDetail(entry.detailAt(&onlineAccounts, 0)));
    QCOMPARE(entry.detailCount(&onlineAccounts), 1);
    QCOMPARE(entry.contact().details<QContactOnlineAccount>().count(), 0);
}

void ContactEntryTest::testRevertChanges()
{
    ContactEntry entry;
    ContactPhoneNumber phoneNumber;
    ContactAddress address;
    ContactOnlineAccount onlineAccount;
    phoneNumber.setNumber("12345");
    address.setStreet("Some Street");
    onlineAccount.setAccountUri("someaccount@someservice");
    QVERIFY(entry.addDetail(&phoneNumber));
    QVERIFY(entry.addDetail(&address));
    QVERIFY(entry.addDetail(&onlineAccount));
    contactModel->saveContact(&entry);

    QModelIndex index = contactModel->index(0);
    ContactEntry *savedEntry = qobject_cast<ContactEntry*>(contactModel->data(index, ContactModel::ContactRole).value<QObject*>());
    QVERIFY(savedEntry);
    int detailCount = savedEntry->contact().details().count();
    QVERIFY(savedEntry->removeDetail(&phoneNumber));
    QVERIFY(savedEntry->removeDetail(&address));
    QVERIFY(savedEntry->removeDetail(&onlineAccount));
    QVERIFY(savedEntry->contact().details().count() < detailCount);
    savedEntry->revertChanges();
    QCOMPARE(savedEntry->contact().details().count(), detailCount);
}

QTEST_MAIN(ContactEntryTest)
#include "ContactEntryTest.moc"
