/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

#include "contactwatcher.h"
#include "contactutils.h"
#include <QContactName>
#include <QContactAvatar>
#include <QContactPhoneNumber>
#include <QContactExtendedDetail>

QTCONTACTS_USE_NAMESPACE

class ContactWatcherTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testIdentifier();
    void testMatchExistingContact();
    void testMatchNewContact();
    void testMatchContactChanged();
    void testClearAfterContactChanged();
    void testContactRemoval();
    void testClearPhoneNumber();
    void testInteractiveProperty_data();
    void testInteractiveProperty();
    void testLateSearch();
    void testAddressableFields();
    void testExtendedFieldMatch();
    void testSimilarPhoneNumbers();

private:
    QContact createContact(const QString &firstName,
                           const QString &lastName,
                           const QString &avatarUrl,
                           const QStringList &phoneNumbers,
                           const QList<int> &subTypes,
                           const QList<int> &contexts);
    void clearManager();
    QContactManager *mManager;
};

void ContactWatcherTest::initTestCase()
{
    // instanciate the shared manager using the memory backend
    mManager = ContactUtils::sharedManager("memory");
}

void ContactWatcherTest::testIdentifier()
{
    QString identifier("123456");
    ContactWatcher watcher;
    QSignalSpy spy(&watcher, SIGNAL(identifierChanged()));
    watcher.setIdentifier(identifier);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(watcher.identifier(), identifier);
}

void ContactWatcherTest::testMatchExistingContact()
{
    QString identifier("12345");
    QContact contact = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     QStringList() << identifier,
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);
    ContactWatcher watcher;
    watcher.componentComplete();
    QSignalSpy contactIdSpy(&watcher, SIGNAL(contactIdChanged()));
    QSignalSpy aliasSpy(&watcher, SIGNAL(aliasChanged()));
    QSignalSpy avatarSpy(&watcher, SIGNAL(avatarChanged()));
    QSignalSpy detailPropertiesSpy(&watcher, SIGNAL(detailPropertiesChanged()));
    QSignalSpy unknownSpy(&watcher, SIGNAL(isUnknownChanged()));

    // set the phone number and wait for the match to happen
    watcher.setIdentifier(identifier);
    watcher.setAddressableFields(QStringList() << "tel");

    // contact fetching is asynchronous so use QTRY_COMPARE for the first signal spy
    // for the subsequent ones it is fine to use just QCOMPARE
    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(aliasSpy.count(), 1);
    QCOMPARE(avatarSpy.count(), 1);
    QCOMPARE(detailPropertiesSpy.count(), 1);
    QCOMPARE(unknownSpy.count(), 1);

    // and verify that the values are properly set
    QCOMPARE(watcher.contactId(), contact.id().toString());
    QCOMPARE(watcher.alias(), ContactUtils::formatContactName(contact));
    QCOMPARE(watcher.avatar(), contact.detail<QContactAvatar>().imageUrl().toString());
    QCOMPARE(watcher.unwrapIntList(watcher.detailProperties()["phoneNumberContexts"].toList()), contact.detail<QContactPhoneNumber>().contexts());
    QCOMPARE(watcher.unwrapIntList(watcher.detailProperties()["phoneNumberSubTypes"].toList()), contact.detail<QContactPhoneNumber>().subTypes());
    QCOMPARE(watcher.isUnknown(), false);

    clearManager();
}

void ContactWatcherTest::testMatchNewContact()
{
    QString identifier("1234567");
    ContactWatcher watcher;
    watcher.componentComplete();
    QSignalSpy contactIdSpy(&watcher, SIGNAL(contactIdChanged()));
    QSignalSpy aliasSpy(&watcher, SIGNAL(aliasChanged()));
    QSignalSpy avatarSpy(&watcher, SIGNAL(avatarChanged()));
    QSignalSpy detailPropertiesSpy(&watcher, SIGNAL(detailPropertiesChanged()));
    QSignalSpy unknownSpy(&watcher, SIGNAL(isUnknownChanged()));

    watcher.setIdentifier(identifier);
    watcher.setAddressableFields(QStringList() << "tel");

    // now create the contact and wait to see if it gets matched
    QContact contact = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     // just to make it a little more complicated, use a prefixed phone number
                                     QStringList() << identifier.prepend("+1"),
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);

    // contact fetching is asynchronous so use QTRY_COMPARE for the first signal spy
    // for the subsequent ones it is fine to use just QCOMPARE
    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(aliasSpy.count(), 1);
    QCOMPARE(avatarSpy.count(), 1);
    QCOMPARE(detailPropertiesSpy.count(), 1);
    QCOMPARE(unknownSpy.count(), 1);

    // and verify that the values are properly set
    QCOMPARE(watcher.contactId(), contact.id().toString());
    QCOMPARE(watcher.alias(), ContactUtils::formatContactName(contact));
    QCOMPARE(watcher.avatar(), contact.detail<QContactAvatar>().imageUrl().toString());
    QCOMPARE(watcher.unwrapIntList(watcher.detailProperties()["phoneNumberContexts"].toList()), contact.detail<QContactPhoneNumber>().contexts());
    QCOMPARE(watcher.unwrapIntList(watcher.detailProperties()["phoneNumberSubTypes"].toList()), contact.detail<QContactPhoneNumber>().subTypes());
    QCOMPARE(watcher.isUnknown(), false);

    clearManager();
}

void ContactWatcherTest::testMatchContactChanged()
{
    QString identifier("12345");
    QContact contact = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     QStringList() << "456456456", // a different phone number
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);
    ContactWatcher watcher;
    watcher.componentComplete();
    watcher.setIdentifier(identifier);
    watcher.setAddressableFields(QStringList() << "tel");

    QSignalSpy contactIdSpy(&watcher, SIGNAL(contactIdChanged()));
    QSignalSpy aliasSpy(&watcher, SIGNAL(aliasChanged()));
    QSignalSpy avatarSpy(&watcher, SIGNAL(avatarChanged()));
    QSignalSpy detailPropertiesSpy(&watcher, SIGNAL(detailPropertiesChanged()));
    QSignalSpy unknownSpy(&watcher, SIGNAL(isUnknownChanged()));

    // now modify the contact´s phone number so that it matches
    QContactPhoneNumber number = contact.detail<QContactPhoneNumber>();
    number.setNumber(identifier);
    contact.saveDetail(&number);
    mManager->saveContact(&contact);

    // contact fetching is asynchronous so use QTRY_COMPARE for the first signal spy
    // for the subsequent ones it is fine to use just QCOMPARE
    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(aliasSpy.count(), 1);
    QCOMPARE(avatarSpy.count(), 1);
    QCOMPARE(detailPropertiesSpy.count(), 1);
    QCOMPARE(unknownSpy.count(), 1);

    // and verify that the values are properly set
    QCOMPARE(watcher.contactId(), contact.id().toString());
    QCOMPARE(watcher.alias(), ContactUtils::formatContactName(contact));
    QCOMPARE(watcher.avatar(), contact.detail<QContactAvatar>().imageUrl().toString());
    QCOMPARE(watcher.unwrapIntList(watcher.detailProperties()["phoneNumberContexts"].toList()), contact.detail<QContactPhoneNumber>().contexts());
    QCOMPARE(watcher.unwrapIntList(watcher.detailProperties()["phoneNumberSubTypes"].toList()), contact.detail<QContactPhoneNumber>().subTypes());
    QCOMPARE(watcher.isUnknown(), false);

    clearManager();
}

void ContactWatcherTest::testClearAfterContactChanged()
{
    // after modifying a contact, if the phone number doesn´t match anymore, the data should be cleared
    // verify that this happens, but first we need to make sure the match actually happened
    QString identifier("12345");
    QContact contact = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     QStringList() << identifier,
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);
    ContactWatcher watcher;
    watcher.componentComplete();
    QSignalSpy contactIdSpy(&watcher, SIGNAL(contactIdChanged()));

    // set the phone number and wait for the match to happen
    watcher.setIdentifier(identifier);
    watcher.setAddressableFields(QStringList() << "tel");

    // at this point we just need to make sure the contactId is correct, the other fields
    // are tested in a separate test
    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(watcher.contactId(), contact.id().toString());
    contactIdSpy.clear();

    QSignalSpy aliasSpy(&watcher, SIGNAL(aliasChanged()));
    QSignalSpy avatarSpy(&watcher, SIGNAL(avatarChanged()));
    QSignalSpy detailPropertiesSpy(&watcher, SIGNAL(detailPropertiesChanged()));
    QSignalSpy unknownSpy(&watcher, SIGNAL(isUnknownChanged()));

    // now modify the contact´s phone number so that it doesn´t match anymore
    QContactPhoneNumber number = contact.detail<QContactPhoneNumber>();
    number.setNumber("43345476");
    contact.saveDetail(&number);
    mManager->saveContact(&contact);

    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(aliasSpy.count(), 1);
    QCOMPARE(avatarSpy.count(), 1);
    QCOMPARE(detailPropertiesSpy.count(), 1);
    QCOMPARE(unknownSpy.count(), 1);

    // and verify that the values are properly cleared
    QVERIFY(watcher.contactId().isEmpty());
    QVERIFY(watcher.alias().isEmpty());
    QVERIFY(watcher.avatar().isEmpty());
    QVERIFY(watcher.detailProperties().isEmpty());
    QVERIFY(watcher.isUnknown());

    clearManager();
}

void ContactWatcherTest::testContactRemoval()
{
    // after removing a contact, the contact match should be cleared
    // verify that this happens, but first we need to make sure the match actually happened
    QString identifier("12345");
    QContact contact = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     QStringList() << identifier,
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);
    ContactWatcher watcher;
    watcher.componentComplete();
    QSignalSpy contactIdSpy(&watcher, SIGNAL(contactIdChanged()));

    // set the phone number and wait for the match to happen
    watcher.setIdentifier(identifier);
    watcher.setAddressableFields(QStringList() << "tel");

    // at this point we just need to make sure the contactId is correct, the other fields
    // are tested in a separate test
    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(watcher.contactId(), contact.id().toString());
    contactIdSpy.clear();

    QSignalSpy aliasSpy(&watcher, SIGNAL(aliasChanged()));
    QSignalSpy avatarSpy(&watcher, SIGNAL(avatarChanged()));
    QSignalSpy detailPropertiesSpy(&watcher, SIGNAL(detailPropertiesChanged()));
    QSignalSpy unknownSpy(&watcher, SIGNAL(isUnknownChanged()));

    // now remove the contact
    mManager->removeContact(contact.id());

    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(aliasSpy.count(), 1);
    QCOMPARE(avatarSpy.count(), 1);
    QCOMPARE(detailPropertiesSpy.count(), 1);
    QCOMPARE(unknownSpy.count(), 1);

    // and verify that the values are properly cleared
    QVERIFY(watcher.contactId().isEmpty());
    QVERIFY(watcher.alias().isEmpty());
    QVERIFY(watcher.avatar().isEmpty());
    QVERIFY(watcher.detailProperties().isEmpty());
    QVERIFY(watcher.isUnknown());

    clearManager();
}

void ContactWatcherTest::testClearPhoneNumber()
{
    // clearing a phone number should trigger the contact data to be cleared too
    // after removing a contact, the contact match should be cleared
    // verify that this happens, but first we need to make sure the match actually happened
    QString identifier("12345");
    QContact contact = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     QStringList() << identifier,
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);
    ContactWatcher watcher;
    watcher.componentComplete();
    QSignalSpy contactIdSpy(&watcher, SIGNAL(contactIdChanged()));

    // set the phone number and wait for the match to happen
    watcher.setIdentifier(identifier);
    watcher.setAddressableFields(QStringList() << "tel");

    // at this point we just need to make sure the contactId is correct, the other fields
    // are tested in a separate test
    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(watcher.contactId(), contact.id().toString());
    contactIdSpy.clear();

    QSignalSpy aliasSpy(&watcher, SIGNAL(aliasChanged()));
    QSignalSpy avatarSpy(&watcher, SIGNAL(avatarChanged()));
    QSignalSpy detailPropertiesSpy(&watcher, SIGNAL(detailPropertiesChanged()));
    QSignalSpy unknownSpy(&watcher, SIGNAL(isUnknownChanged()));

    // now clear the phone number
    watcher.setIdentifier("");

    QCOMPARE(contactIdSpy.count(), 1);
    QCOMPARE(aliasSpy.count(), 1);
    QCOMPARE(avatarSpy.count(), 1);
    QCOMPARE(detailPropertiesSpy.count(), 1);
    QCOMPARE(unknownSpy.count(), 1);

    // and verify that the values are properly cleared
    QVERIFY(watcher.contactId().isEmpty());
    QVERIFY(watcher.alias().isEmpty());
    QVERIFY(watcher.avatar().isEmpty());
    QVERIFY(watcher.detailProperties().isEmpty());
    QVERIFY(watcher.isUnknown());

    clearManager();
}

void ContactWatcherTest::testInteractiveProperty_data()
{
    QTest::addColumn<QString>("identifier");
    QTest::addColumn<int>("signalCount");
    QTest::addColumn<bool>("interactive");

    QTest::newRow("valid phone number") << "98765432" << 1 << true;
    QTest::newRow("ofono private phone number") << OFONO_PRIVATE_NUMBER << 0 << false;
    QTest::newRow("ofono unknown number") << OFONO_UNKNOWN_NUMBER << 0 << false;
    QTest::newRow("empty phone number") << "" << 0 << false;
}

void ContactWatcherTest::testInteractiveProperty()
{
    QFETCH(QString, identifier);
    QFETCH(int, signalCount);
    QFETCH(bool, interactive);

    ContactWatcher watcher;
    watcher.componentComplete();
    QSignalSpy spy(&watcher, SIGNAL(interactiveChanged()));

    watcher.setIdentifier(identifier);
    // the initial interactive value is false it will not change in case of invalid phones
    QTRY_COMPARE(spy.count(), signalCount);
    QCOMPARE(watcher.interactive(), interactive);
}


void ContactWatcherTest::testLateSearch()
{
    QString identifier("12345");
    QContact contact = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     QStringList() << identifier,
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);
    ContactWatcher watcher;
    QSignalSpy contactIdSpy(&watcher, SIGNAL(contactIdChanged()));
    QSignalSpy aliasSpy(&watcher, SIGNAL(aliasChanged()));
    QSignalSpy avatarSpy(&watcher, SIGNAL(avatarChanged()));
    QSignalSpy detailPropertiesSpy(&watcher, SIGNAL(detailPropertiesChanged()));
    QSignalSpy unknownSpy(&watcher, SIGNAL(isUnknownChanged()));

    // set the phone number and wait for the match to happen
    watcher.setIdentifier(identifier);
    watcher.setAddressableFields(QStringList() << "tel");

    // component not complete yet
    QTRY_COMPARE(contactIdSpy.count(), 0);
    QCOMPARE(aliasSpy.count(), 0);
    QCOMPARE(avatarSpy.count(), 0);
    QCOMPARE(detailPropertiesSpy.count(), 0);
    QCOMPARE(unknownSpy.count(), 0);

    // mark as complete
    watcher.componentComplete();

    // signal will be fired now
    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(aliasSpy.count(), 1);
    QCOMPARE(avatarSpy.count(), 1);
    QCOMPARE(detailPropertiesSpy.count(), 1);
    QCOMPARE(unknownSpy.count(), 1);

    // and verify that the values are properly set
    QCOMPARE(watcher.contactId(), contact.id().toString());
    QCOMPARE(watcher.alias(), ContactUtils::formatContactName(contact));
    QCOMPARE(watcher.avatar(), contact.detail<QContactAvatar>().imageUrl().toString());
    QCOMPARE(watcher.unwrapIntList(watcher.detailProperties()["phoneNumberContexts"].toList()), contact.detail<QContactPhoneNumber>().contexts());
    QCOMPARE(watcher.unwrapIntList(watcher.detailProperties()["phoneNumberSubTypes"].toList()), contact.detail<QContactPhoneNumber>().subTypes());
    QCOMPARE(watcher.isUnknown(), false);

    clearManager();
}

void ContactWatcherTest::testAddressableFields()
{
    ContactWatcher watcher;

    // check that addressable fields contains "tel" by default
    QCOMPARE(watcher.addressableFields().count(), 0);
    QCOMPARE(watcher.addressableFields(), QStringList());

    QSignalSpy addressableFieldsSpy(&watcher, SIGNAL(addressableFieldsChanged()));
    QStringList addressableFields;
    addressableFields << "x-jabber" << "tel" << "x-sip";
    watcher.setAddressableFields(addressableFields);
    QCOMPARE(addressableFieldsSpy.count(), 1);
    QCOMPARE(watcher.addressableFields(), addressableFields);

    // set the addressable fields to an empty value and make sure it falls back to "tel"
    watcher.setAddressableFields(QStringList());
    QCOMPARE(watcher.addressableFields().count(), 0);
    QCOMPARE(watcher.addressableFields(), QStringList());
}

void ContactWatcherTest::testExtendedFieldMatch()
{
    QString field("x-jabber");
    QString identifier("foo.bar@someserver.jabber");
    QContact contact = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     QStringList() << "12345",
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);

    // now add the extended info to the contact
    QContactExtendedDetail detail;
    detail.setName(field);
    detail.setData(identifier);
    contact.appendDetail(detail);
    mManager->saveContact(&contact);

    // now create the watcher and check that it matches this field
    ContactWatcher watcher;
    QSignalSpy contactIdSpy(&watcher, SIGNAL(contactIdChanged()));
    watcher.setIdentifier(identifier);
    watcher.setAddressableFields(QStringList() << field);
    watcher.componentComplete();

    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(watcher.contactId(), contact.id().toString());
}

void ContactWatcherTest::testSimilarPhoneNumbers()
{
    QString contactIdentifierA("+352 661 123456");
    QString contactIdentifierB("+352 691 123456");
    QContact contactA = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     QStringList() << contactIdentifierA,
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);
    QContact contactB = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     QStringList() << contactIdentifierB,
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);
    ContactWatcher watcherA;
    QSignalSpy contactIdSpyA(&watcherA, SIGNAL(contactIdChanged()));

    // try to match contact A
    watcherA.setIdentifier(contactIdentifierA);
    watcherA.setAddressableFields(QStringList() << "tel");

    // mark as complete
    watcherA.componentComplete();

    // signal will be fired now
    QTRY_COMPARE(contactIdSpyA.count(), 1);
 
    QCOMPARE(watcherA.contactId(), contactA.id().toString());

    ContactWatcher watcherB;
    QSignalSpy contactIdSpyB(&watcherB, SIGNAL(contactIdChanged()));

    // mark as complete
    watcherB.componentComplete();

    // try to match contact B
    watcherB.setIdentifier(contactIdentifierB);
    watcherB.setAddressableFields(QStringList() << "tel");

    // signal will be fired now
    QTRY_COMPARE(contactIdSpyB.count(), 1);
 
    QCOMPARE(watcherB.contactId(), contactB.id().toString());
}

QContact ContactWatcherTest::createContact(const QString &firstName,
                                           const QString &lastName,
                                           const QString &avatarUrl,
                                           const QStringList &phoneNumbers,
                                           const QList<int> &subTypes,
                                           const QList<int> &contexts)
{
    QContact contact;

    // Name
    QContactName name;
    name.setFirstName(firstName);
    name.setLastName(lastName);
    contact.saveDetail(&name);

    // Avatar
    QContactAvatar avatar;
    avatar.setImageUrl(avatarUrl);
    contact.saveDetail(&avatar);

    Q_FOREACH(const QString &phoneNumber, phoneNumbers) {
        QContactPhoneNumber number;
        number.setNumber(phoneNumber);
        number.setSubTypes(subTypes);
        number.setContexts(contexts);
        contact.saveDetail(&number);
    }

    mManager->saveContact(&contact);
    return contact;
}

void ContactWatcherTest::clearManager()
{
    Q_FOREACH(QContact contact, mManager->contacts()) {
        mManager->removeContact(contact.id());
    }
}

QTEST_MAIN(ContactWatcherTest)
#include "ContactWatcherTest.moc"
