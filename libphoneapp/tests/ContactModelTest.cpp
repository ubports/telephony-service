/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
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
#include <QContactAvatar>
#include <QContactGuid>
#include <QContactName>
#include <QContactPhoneNumber>
#include "contactmodel.h"
#include "contactentry.h"

QTCONTACTS_USE_NAMESPACE

Q_DECLARE_METATYPE(ContactEntry)
Q_DECLARE_METATYPE(ContactEntry*)

class ContactModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testRowCount();
    void testData();
    void testContactFromId();
    void testContactFromPhoneNumber();
    void testComparePhoneNumbers_data();
    void testComparePhoneNumbers();
    void testRemoveContact();
    void testContactAddedSignal();
    void testContactChangedSignal();
    void testContactRemovedSignal();
    void testContactLoadedSignal();
    void testContactSavedSignal();

private:
    QContactManager *contactManager;
    ContactModel *contactModel;
};

void ContactModelTest::initTestCase()
{
    contactModel = ContactModel::instance("memory");
    contactManager = contactModel->contactManager();

    qRegisterMetaType<ContactEntry>();
    qRegisterMetaType<ContactEntry*>();
}

void ContactModelTest::cleanupTestCase()
{
    delete contactManager;
}

void ContactModelTest::testRowCount()
{
    int rowCount = contactModel->rowCount();

    // add contact
    QContact contact;
    QVERIFY(contactManager->saveContact(&contact));
    QCOMPARE(contactModel->rowCount(), rowCount+1);

    // the rowcount for child items needs to be zero
    QCOMPARE(contactModel->rowCount(contactModel->index(0)), 0);

    // then remove the contact
    QVERIFY(contactManager->removeContact(contact.id()));
    QCOMPARE(contactModel->rowCount(), rowCount);
}

void ContactModelTest::testData()
{
    QSignalSpy signalSpy(contactModel, SIGNAL(rowsInserted(QModelIndex, int, int)));

    // insert a few contacts and make sure the data() method returns the correct information
    for (int i = 0; i < 5; ++i) {
        QContact contact;
        QContactName nameDetail;
        QContactDisplayLabel labelDetail;
        QContactAvatar avatarDetail;
        nameDetail.setFirstName(QString("FirstName%1").arg(i));
        nameDetail.setLastName(QString("LastName%1").arg(i));
        QVERIFY(contact.saveDetail(&nameDetail));
        labelDetail.setLabel(QString("%1 %2").arg(nameDetail.firstName()).arg(nameDetail.lastName()));
        QVERIFY(contact.saveDetail(&labelDetail));
        avatarDetail.setImageUrl(QUrl::fromLocalFile(QString("/fake/path/for/contact/%1.png").arg(i)));
        QVERIFY(contact.saveDetail(&avatarDetail));
        QVERIFY(contactManager->saveContact(&contact));

        QVERIFY(signalSpy.count() > i);
        int row = signalSpy.last()[1].toInt();
        QModelIndex index = contactModel->index(row);

        QString displayLabel = contactModel->data(index, Qt::DisplayRole).toString();
        QCOMPARE(displayLabel, labelDetail.label());

        QUrl avatarUrl = contactModel->data(index, Qt::DecorationRole).toUrl();
        QCOMPARE(avatarUrl, avatarDetail.imageUrl());

        ContactEntry *entry = qobject_cast<ContactEntry*>(contactModel->data(index, ContactModel::ContactRole).value<QObject*>());
        QVERIFY(entry);
        QCOMPARE(entry->contact(), contact);

        QString initial = contactModel->data(index, ContactModel::InitialRole).toString();
        QCOMPARE(entry->initial(), initial);
    }
}

void ContactModelTest::testContactFromId()
{
    // FIXME: check how to properly set the id for comparision in Qt5
#if 0
    QContact contact;
    QContactGuid guidDetail;
    guidDetail.setGuid("testcontactfromid");
    QVERIFY(contact.saveDetail(&guidDetail));
    QVERIFY(contactManager->saveContact(&contact));
    ContactEntry *entry = contactModel->contactFromId(guidDetail.guid());
    QVERIFY(entry);
    QCOMPARE(entry->id(), guidDetail.guid());
    QCOMPARE(entry->contact(), contact);
#endif
}

void ContactModelTest::testContactFromPhoneNumber()
{
    QContact contact;
    QContactPhoneNumber phoneNumberDetail;
    phoneNumberDetail.setNumber("12345678");
    QVERIFY(contact.saveDetail(&phoneNumberDetail));
    QVERIFY(contactManager->saveContact(&contact));
    ContactEntry *entry = contactModel->contactFromPhoneNumber(phoneNumberDetail.number());
    QVERIFY(entry);
    QCOMPARE(entry->contact().detail<QContactPhoneNumber>().number(), phoneNumberDetail.number());
    QCOMPARE(entry->contact(), contact);

    // remove the contact not to mess with other tests using phone numbers
    QVERIFY(contactManager->removeContact(contact.id()));
}

void ContactModelTest::testComparePhoneNumbers_data()
{
    QTest::addColumn<QString>("number1");
    QTest::addColumn<QString>("number2");
    QTest::addColumn<bool>("expectedResult");

    QTest::newRow("string equal") << "12345678" << "12345678" << true;
    QTest::newRow("number with dash") << "1234-5678" << "12345678" << true;
    QTest::newRow("number with area code") << "12312345678" << "12345678" << true;
    QTest::newRow("number with extension") << "12345678#123" << "12345678" << false;
    QTest::newRow("both numbers with extension") << "(123)12345678#1" << "12345678#1" << true;
    QTest::newRow("numbers with different extension") << "1234567#1" << "1234567#2" << false;
    QTest::newRow("number with comma") << "33333333,1,1" << "33333333" << true;
    QTest::newRow("both numbers with comma") << "22222222,1" << "22222222,2,1" << true;
    QTest::newRow("number with semicolon") << "33333333;1" << "33333333" << true;
    QTest::newRow("both numbers with semicolon") << "22222222;1" << "22222222;2" << true;
    QTest::newRow("short/emergency numbers") << "190" << "190" << true;
    QTest::newRow("different numbers") << "12345678" << "1234567" << false;
    // FIXME: check what other cases we need to test here"
}

void ContactModelTest::testComparePhoneNumbers()
{
    QFETCH(QString, number1);
    QFETCH(QString, number2);
    QFETCH(bool, expectedResult);

    bool result = ContactModel::comparePhoneNumbers(number1, number2);
    QCOMPARE(result, expectedResult);
}

void ContactModelTest::testRemoveContact()
{
    // FIXME: now that the contact id is set in the manager engine itself, we need
    // to find a way to properly emulate that for testing.
#if 0
    QSignalSpy signalSpy(contactModel, SIGNAL(contactRemoved(QString)));

    QContact contact;
    QContactGuid guidDetail;
    QString id("testremovecontact");
    guidDetail.setGuid(id);
    QVERIFY(contact.saveDetail(&guidDetail));
    QVERIFY(contactManager->saveContact(&contact));

    ContactEntry *entry = contactModel->contactFromId(id);
    QVERIFY(entry);

    contactModel->removeContact(entry);
    // the contact removal happens asynchronously so we need to wait a bit
    // in case the operation is not yet finished
    int tries = 0;
    while (signalSpy.count() == 0) {
        QTest::qWait(100);
        tries++;
        if (tries == 5) {
            break;
        }
    }

    QCOMPARE(signalSpy.count(), 1);
    QCOMPARE(signalSpy[0][0].toString(), id);
#endif
}

void ContactModelTest::testContactAddedSignal()
{
    QSignalSpy signalSpy(contactModel, SIGNAL(contactAdded(ContactEntry*)));

    QContact contact;
    QVERIFY(contactManager->saveContact(&contact));
    QCOMPARE(signalSpy.count(), 1);

    ContactEntry *entry = signalSpy.at(0).at(0).value<ContactEntry*>();
    QCOMPARE(entry->contact(), contact);
}

void ContactModelTest::testContactChangedSignal()
{
    QSignalSpy signalSpy(contactModel, SIGNAL(contactChanged(ContactEntry*)));

    QContact contact;
    QVERIFY(contactManager->saveContact(&contact));

    QContactName name;
    QString firstName("Test");
    name.setFirstName(firstName);
    QVERIFY(contact.saveDetail(&name));
    QVERIFY(contactManager->saveContact(&contact));

    QCOMPARE(signalSpy.count(), 1);
    ContactEntry *entry = signalSpy.at(0).at(0).value<ContactEntry*>();
    QCOMPARE(entry->contact(), contact);
    QCOMPARE(entry->contact().detail<QContactName>().firstName(), firstName);
}

void ContactModelTest::testContactRemovedSignal()
{
    // FIXME: now that the contact id is set in the manager engine itself, we need
    // to find a way to properly emulate that for testing.
#if 0
    QSignalSpy signalSpy(contactModel, SIGNAL(contactRemoved(QString)));

    QString id("contact1");
    QContact contact;
    QContactGuid guid;
    // the custom id details holds all the backend IDs, but we are just using the last one
    guid.setGuid(id);
    contact.saveDetail(&guid);
    QVERIFY(contactManager->saveContact(&contact));
    QVERIFY(contactManager->removeContact(contact.id()));

    QCOMPARE(signalSpy.count(), 1);
    QString removedId = signalSpy.at(0).at(0).toString();
    QCOMPARE(removedId, id);
#endif
}

void ContactModelTest::testContactLoadedSignal()
{
    // FIXME: now that the contact id is set in the manager engine itself, we need
    // to find a way to properly emulate that for testing.
#if 0
    // the contactLoaded signal is emitted as a result of calling
    // ContactModel::loadContactFromId(). If the contact is not yet loaded
    // this signal needs to be emitted once the contact is loaded
    QSignalSpy signalSpy(contactModel, SIGNAL(contactLoaded(ContactEntry*)));
    QString id("asyncLoadContact1");
    contactModel->loadContactFromId(id);

    QContact contact;
    QContactGuid guid;
    guid.setGuid(id);
    QVERIFY(contact.saveDetail(&guid));
    QVERIFY(contactManager->saveContact(&contact));
    // at this point the contactLoaded signal should have been already emitted for
    // the requested id
    QCOMPARE(signalSpy.count(), 1);
    ContactEntry *entry = signalSpy.at(0).at(0).value<ContactEntry*>();
    QCOMPARE(entry->contact().detail<QContactGuid>().guid(), id);

    // now we try to call the function again to make sure the signal gets emitted
    // for a contact that is already there
    contactModel->loadContactFromId(id);
    QCOMPARE(signalSpy.count(), 2);
    entry = signalSpy.at(1).at(0).value<ContactEntry*>();
    QCOMPARE(entry->contact().detail<QContactGuid>().guid(), id);
#endif
}

void ContactModelTest::testContactSavedSignal()
{
#if 0
    QSignalSpy signalSpy(contactModel, SIGNAL(contactSaved(QString)));
    QString guid("contactsavedguid");

    QContact contact;
    QContactGuid guidDetail;
    guidDetail.setGuid(guid);
    QVERIFY(contact.saveDetail(&guidDetail));

    ContactEntry entry(contact);
    entry.setModified(true);
    contactModel->saveContact(&entry);

    // the contact save happens asynchronously so we need to wait a bit
    // in case the operation is not yet finished
    int tries = 0;
    while (signalSpy.count() == 0) {
        QTest::qWait(100);
        tries++;
        if (tries == 5) {
            break;
        }
    }

    QCOMPARE(signalSpy.count(), 1);
    QString savedGuid = signalSpy.at(0).at(0).toString();
    QCOMPARE(savedGuid, guid);
#endif
}

QTEST_MAIN(ContactModelTest)
#include "ContactModelTest.moc"
