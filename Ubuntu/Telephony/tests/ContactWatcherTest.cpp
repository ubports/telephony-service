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

#include "contactwatcher.h"
#include "contactutils.h"
#include <QContactName>
#include <QContactAvatar>
#include <QContactPhoneNumber>

QTCONTACTS_USE_NAMESPACE

class ContactWatcherTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testPhoneNumber();
    void testMatchExistingContact();
    void testMatchNewContact();
    void testMatchContactChanged();
    void testClearAfterContactChanged();
    void testContactRemoval();
    void testClearPhoneNumber();
    void testInteractiveProperty_data();
    void testInteractiveProperty();

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

void ContactWatcherTest::testPhoneNumber()
{
    QString phoneNumber("123456");
    ContactWatcher watcher;
    QSignalSpy spy(&watcher, SIGNAL(phoneNumberChanged()));
    watcher.setPhoneNumber(phoneNumber);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(watcher.phoneNumber(), phoneNumber);
}

void ContactWatcherTest::testMatchExistingContact()
{
    QString phoneNumber("12345");
    QContact contact = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     QStringList() << phoneNumber,
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);
    ContactWatcher watcher;
    QSignalSpy contactIdSpy(&watcher, SIGNAL(contactIdChanged()));
    QSignalSpy aliasSpy(&watcher, SIGNAL(aliasChanged()));
    QSignalSpy avatarSpy(&watcher, SIGNAL(avatarChanged()));
    QSignalSpy contextsSpy(&watcher, SIGNAL(phoneNumberContextsChanged()));
    QSignalSpy subTypesSpy(&watcher, SIGNAL(phoneNumberSubTypesChanged()));
    QSignalSpy unknownSpy(&watcher, SIGNAL(isUnknownChanged()));

    // set the phone number and wait for the match to happen
    watcher.setPhoneNumber(phoneNumber);

    // contact fetching is asynchronous so use QTRY_COMPARE for the first signal spy
    // for the subsequent ones it is fine to use just QCOMPARE
    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(aliasSpy.count(), 1);
    QCOMPARE(avatarSpy.count(), 1);
    QCOMPARE(contextsSpy.count(), 1);
    QCOMPARE(subTypesSpy.count(), 1);
    QCOMPARE(unknownSpy.count(), 1);

    // and verify that the values are properly set
    QCOMPARE(watcher.contactId(), contact.id().toString());
    QCOMPARE(watcher.alias(), ContactUtils::formatContactName(contact));
    QCOMPARE(watcher.avatar(), contact.detail<QContactAvatar>().imageUrl().toString());
    QCOMPARE(watcher.phoneNumberContexts(), contact.detail<QContactPhoneNumber>().contexts());
    QCOMPARE(watcher.phoneNumberSubTypes(), contact.detail<QContactPhoneNumber>().subTypes());
    QCOMPARE(watcher.isUnknown(), false);

    clearManager();
}

void ContactWatcherTest::testMatchNewContact()
{
    QString phoneNumber("1234567");
    ContactWatcher watcher;
    QSignalSpy contactIdSpy(&watcher, SIGNAL(contactIdChanged()));
    QSignalSpy aliasSpy(&watcher, SIGNAL(aliasChanged()));
    QSignalSpy avatarSpy(&watcher, SIGNAL(avatarChanged()));
    QSignalSpy contextsSpy(&watcher, SIGNAL(phoneNumberContextsChanged()));
    QSignalSpy subTypesSpy(&watcher, SIGNAL(phoneNumberSubTypesChanged()));
    QSignalSpy unknownSpy(&watcher, SIGNAL(isUnknownChanged()));

    watcher.setPhoneNumber(phoneNumber);

    // now create the contact and wait to see if it gets matched
    QContact contact = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     // just to make it a little more complicated, use a prefixed phone number
                                     QStringList() << phoneNumber.prepend("+1"),
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);

    // contact fetching is asynchronous so use QTRY_COMPARE for the first signal spy
    // for the subsequent ones it is fine to use just QCOMPARE
    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(aliasSpy.count(), 1);
    QCOMPARE(avatarSpy.count(), 1);
    QCOMPARE(contextsSpy.count(), 1);
    QCOMPARE(subTypesSpy.count(), 1);
    QCOMPARE(unknownSpy.count(), 1);

    // and verify that the values are properly set
    QCOMPARE(watcher.contactId(), contact.id().toString());
    QCOMPARE(watcher.alias(), ContactUtils::formatContactName(contact));
    QCOMPARE(watcher.avatar(), contact.detail<QContactAvatar>().imageUrl().toString());
    QCOMPARE(watcher.phoneNumberContexts(), contact.detail<QContactPhoneNumber>().contexts());
    QCOMPARE(watcher.phoneNumberSubTypes(), contact.detail<QContactPhoneNumber>().subTypes());
    QCOMPARE(watcher.isUnknown(), false);

    clearManager();
}

void ContactWatcherTest::testMatchContactChanged()
{
    QString phoneNumber("12345");
    QContact contact = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     QStringList() << "456456456", // a different phone number
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);
    ContactWatcher watcher;
    watcher.setPhoneNumber(phoneNumber);

    QSignalSpy contactIdSpy(&watcher, SIGNAL(contactIdChanged()));
    QSignalSpy aliasSpy(&watcher, SIGNAL(aliasChanged()));
    QSignalSpy avatarSpy(&watcher, SIGNAL(avatarChanged()));
    QSignalSpy contextsSpy(&watcher, SIGNAL(phoneNumberContextsChanged()));
    QSignalSpy subTypesSpy(&watcher, SIGNAL(phoneNumberSubTypesChanged()));
    QSignalSpy unknownSpy(&watcher, SIGNAL(isUnknownChanged()));

    // now modify the contact´s phone number so that it matches
    QContactPhoneNumber number = contact.detail<QContactPhoneNumber>();
    number.setNumber(phoneNumber);
    contact.saveDetail(&number);
    mManager->saveContact(&contact);

    // contact fetching is asynchronous so use QTRY_COMPARE for the first signal spy
    // for the subsequent ones it is fine to use just QCOMPARE
    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(aliasSpy.count(), 1);
    QCOMPARE(avatarSpy.count(), 1);
    QCOMPARE(contextsSpy.count(), 1);
    QCOMPARE(subTypesSpy.count(), 1);
    QCOMPARE(unknownSpy.count(), 1);

    // and verify that the values are properly set
    QCOMPARE(watcher.contactId(), contact.id().toString());
    QCOMPARE(watcher.alias(), ContactUtils::formatContactName(contact));
    QCOMPARE(watcher.avatar(), contact.detail<QContactAvatar>().imageUrl().toString());
    QCOMPARE(watcher.phoneNumberContexts(), contact.detail<QContactPhoneNumber>().contexts());
    QCOMPARE(watcher.phoneNumberSubTypes(), contact.detail<QContactPhoneNumber>().subTypes());
    QCOMPARE(watcher.isUnknown(), false);

    clearManager();
}

void ContactWatcherTest::testClearAfterContactChanged()
{
    // after modifying a contact, if the phone number doesn´t match anymore, the data should be cleared
    // verify that this happens, but first we need to make sure the match actually happened
    QString phoneNumber("12345");
    QContact contact = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     QStringList() << phoneNumber,
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);
    ContactWatcher watcher;
    QSignalSpy contactIdSpy(&watcher, SIGNAL(contactIdChanged()));

    // set the phone number and wait for the match to happen
    watcher.setPhoneNumber(phoneNumber);

    // at this point we just need to make sure the contactId is correct, the other fields
    // are tested in a separate test
    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(watcher.contactId(), contact.id().toString());
    contactIdSpy.clear();

    QSignalSpy aliasSpy(&watcher, SIGNAL(aliasChanged()));
    QSignalSpy avatarSpy(&watcher, SIGNAL(avatarChanged()));
    QSignalSpy contextsSpy(&watcher, SIGNAL(phoneNumberContextsChanged()));
    QSignalSpy subTypesSpy(&watcher, SIGNAL(phoneNumberSubTypesChanged()));
    QSignalSpy unknownSpy(&watcher, SIGNAL(isUnknownChanged()));

    // now modify the contact´s phone number so that it doesn´t match anymore
    QContactPhoneNumber number = contact.detail<QContactPhoneNumber>();
    number.setNumber("43345476");
    contact.saveDetail(&number);
    mManager->saveContact(&contact);

    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(aliasSpy.count(), 1);
    QCOMPARE(avatarSpy.count(), 1);
    QCOMPARE(contextsSpy.count(), 1);
    QCOMPARE(subTypesSpy.count(), 1);
    QCOMPARE(unknownSpy.count(), 1);

    // and verify that the values are properly cleared
    QCOMPARE(watcher.contactId(), QString(""));
    QCOMPARE(watcher.alias(), QString(""));
    QCOMPARE(watcher.avatar(), QString(""));
    QCOMPARE(watcher.phoneNumberContexts().count(), 0);
    QCOMPARE(watcher.phoneNumberSubTypes().count(), 0);
    QCOMPARE(watcher.isUnknown(), true);

    clearManager();
}

void ContactWatcherTest::testContactRemoval()
{
    // after removing a contact, the contact match should be cleared
    // verify that this happens, but first we need to make sure the match actually happened
    QString phoneNumber("12345");
    QContact contact = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     QStringList() << phoneNumber,
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);
    ContactWatcher watcher;
    QSignalSpy contactIdSpy(&watcher, SIGNAL(contactIdChanged()));

    // set the phone number and wait for the match to happen
    watcher.setPhoneNumber(phoneNumber);

    // at this point we just need to make sure the contactId is correct, the other fields
    // are tested in a separate test
    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(watcher.contactId(), contact.id().toString());
    contactIdSpy.clear();

    QSignalSpy aliasSpy(&watcher, SIGNAL(aliasChanged()));
    QSignalSpy avatarSpy(&watcher, SIGNAL(avatarChanged()));
    QSignalSpy contextsSpy(&watcher, SIGNAL(phoneNumberContextsChanged()));
    QSignalSpy subTypesSpy(&watcher, SIGNAL(phoneNumberSubTypesChanged()));
    QSignalSpy unknownSpy(&watcher, SIGNAL(isUnknownChanged()));

    // now remove the contact
    mManager->removeContact(contact.id());

    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(aliasSpy.count(), 1);
    QCOMPARE(avatarSpy.count(), 1);
    QCOMPARE(contextsSpy.count(), 1);
    QCOMPARE(subTypesSpy.count(), 1);
    QCOMPARE(unknownSpy.count(), 1);

    // and verify that the values are properly cleared
    QCOMPARE(watcher.contactId(), QString(""));
    QCOMPARE(watcher.alias(), QString(""));
    QCOMPARE(watcher.avatar(), QString(""));
    QCOMPARE(watcher.phoneNumberContexts().count(), 0);
    QCOMPARE(watcher.phoneNumberSubTypes().count(), 0);
    QCOMPARE(watcher.isUnknown(), true);

    clearManager();
}

void ContactWatcherTest::testClearPhoneNumber()
{
    // clearing a phone number should trigger the contact data to be cleared too
    // after removing a contact, the contact match should be cleared
    // verify that this happens, but first we need to make sure the match actually happened
    QString phoneNumber("12345");
    QContact contact = createContact("FirstName",
                                     "LastName",
                                     "file://some_file",
                                     QStringList() << phoneNumber,
                                     QList<int>() << 0 << 1 << 2,
                                     QList<int>() << 3 << 4 << 5);
    ContactWatcher watcher;
    QSignalSpy contactIdSpy(&watcher, SIGNAL(contactIdChanged()));

    // set the phone number and wait for the match to happen
    watcher.setPhoneNumber(phoneNumber);

    // at this point we just need to make sure the contactId is correct, the other fields
    // are tested in a separate test
    QTRY_COMPARE(contactIdSpy.count(), 1);
    QCOMPARE(watcher.contactId(), contact.id().toString());
    contactIdSpy.clear();

    QSignalSpy aliasSpy(&watcher, SIGNAL(aliasChanged()));
    QSignalSpy avatarSpy(&watcher, SIGNAL(avatarChanged()));
    QSignalSpy contextsSpy(&watcher, SIGNAL(phoneNumberContextsChanged()));
    QSignalSpy subTypesSpy(&watcher, SIGNAL(phoneNumberSubTypesChanged()));
    QSignalSpy unknownSpy(&watcher, SIGNAL(isUnknownChanged()));

    // now clear the phone number
    watcher.setPhoneNumber("");

    QCOMPARE(contactIdSpy.count(), 1);
    QCOMPARE(aliasSpy.count(), 1);
    QCOMPARE(avatarSpy.count(), 1);
    QCOMPARE(contextsSpy.count(), 1);
    QCOMPARE(subTypesSpy.count(), 1);
    QCOMPARE(unknownSpy.count(), 1);

    // and verify that the values are properly cleared
    QCOMPARE(watcher.contactId(), QString(""));
    QCOMPARE(watcher.alias(), QString(""));
    QCOMPARE(watcher.avatar(), QString(""));
    QCOMPARE(watcher.phoneNumberContexts().count(), 0);
    QCOMPARE(watcher.phoneNumberSubTypes().count(), 0);
    QCOMPARE(watcher.isUnknown(), true);

    clearManager();
}

void ContactWatcherTest::testInteractiveProperty_data()
{
    QTest::addColumn<QString>("phoneNumber");
    QTest::addColumn<bool>("interactive");

    QTest::newRow("valid phone number") << "98765432" << true;
    QTest::newRow("ofono private phone number") << "x-ofono-private" << false;
    QTest::newRow("ofono unknown number") << "x-ofono-unknown" << false;
    QTest::newRow("empty phone number") << "" << false;
}

void ContactWatcherTest::testInteractiveProperty()
{
    QFETCH(QString, phoneNumber);
    QFETCH(bool, interactive);

    ContactWatcher watcher;
    QSignalSpy spy(&watcher, SIGNAL(interactiveChanged()));

    watcher.setPhoneNumber(phoneNumber);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(watcher.interactive(), interactive);
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
