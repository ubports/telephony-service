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
#include <QContactAvatar>
#include <QContactDetailDefinition>
#include <QContactGuid>
#include <QContactName>
#include <QContactPhoneNumber>
#include "contactmodel.h"
#include "contactentry.h"
#include "contactcustomid.h"

using namespace QtMobility;

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
    void testContactFromCustomId();
    void testContactFromPhoneNumber();
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

    // register the ContactCustomId detail definition
    QContactDetailDefinition definition;
    definition.setName("CustomId");

    QContactDetailFieldDefinition fieldDefinition;
    fieldDefinition.setDataType(QVariant::String);
    definition.insertField(ContactCustomId::FieldCustomId, fieldDefinition);
    QVERIFY(contactManager->saveDetailDefinition(definition));

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
    QVERIFY(contactManager->removeContact(contact.localId()));
    QCOMPARE(contactModel->rowCount(), rowCount);
}

void ContactModelTest::testData()
{
    QSignalSpy signalSpy(contactModel, SIGNAL(rowsInserted(QModelIndex, int, int)));

    // insert a few contacts and make sure the data() method returns the correct information
    for (int i = 0; i < 5; ++i) {
        QContact contact;
        QContactName nameDetail;
        QContactAvatar avatarDetail;
        nameDetail.setFirstName(QString("FirstName%1").arg(i));
        nameDetail.setLastName(QString("LastName%1").arg(i));
        nameDetail.setCustomLabel(QString("%1 %2").arg(nameDetail.firstName()).arg(nameDetail.lastName()));
        QVERIFY(contact.saveDetail(&nameDetail));
        avatarDetail.setImageUrl(QUrl::fromLocalFile(QString("/fake/path/for/contact/%1.png").arg(i)));
        QVERIFY(contact.saveDetail(&avatarDetail));
        QVERIFY(contactManager->saveContact(&contact));

        QVERIFY(signalSpy.count() > i);
        int row = signalSpy.last()[1].toInt();
        QModelIndex index = contactModel->index(row);

        QString displayLabel = contactModel->data(index, Qt::DisplayRole).toString();
        QCOMPARE(displayLabel, contact.displayLabel());

        QUrl avatarUrl = contactModel->data(index, Qt::DecorationRole).toUrl();
        QCOMPARE(avatarUrl, avatarDetail.imageUrl());

        ContactEntry *entry = qobject_cast<ContactEntry*>(contactModel->data(index, ContactModel::ContactRole).value<QObject*>());
        QVERIFY(entry);
        QCOMPARE(entry->contact(), contact);
    }
}

void ContactModelTest::testContactFromId()
{
    QContact contact;
    QContactGuid guidDetail;
    guidDetail.setGuid("testcontactfromid");
    QVERIFY(contact.saveDetail(&guidDetail));
    QVERIFY(contactManager->saveContact(&contact));
    ContactEntry *entry = contactModel->contactFromId(guidDetail.guid());
    QVERIFY(entry);
    QCOMPARE(entry->id(), guidDetail.guid());
    QCOMPARE(entry->contact(), contact);
}

void ContactModelTest::testContactFromCustomId()
{
    QContact contact;
    ContactCustomId customIdDetail;
    QString customId("testcontactfromcustomid");
    customIdDetail.setCustomId(QString("anotherid:%1").arg(customId));
    QVERIFY(contact.saveDetail(&customIdDetail));
    QVERIFY(contactManager->saveContact(&contact));
    ContactEntry *entry = contactModel->contactFromCustomId(customId);
    QVERIFY(entry);
    QCOMPARE(entry->customId(), customId);
    QCOMPARE(entry->contact(), contact);
}

void ContactModelTest::testContactFromPhoneNumber()
{
    QContact contact;
    ContactCustomId customIdDetail;
    QContactPhoneNumber phoneNumberDetail;
    QString customId("testcontactfromphonenumber");
    customIdDetail.setCustomId(QString("anotherid:%1").arg(customId));
    QVERIFY(contact.saveDetail(&customIdDetail));
    phoneNumberDetail.setNumber("12345678");
    QVERIFY(contact.saveDetail(&phoneNumberDetail));
    QVERIFY(contactManager->saveContact(&contact));
    ContactEntry *entry = contactModel->contactFromPhoneNumber(phoneNumberDetail.number());
    QVERIFY(entry);
    QCOMPARE(entry->contact().detail<QContactPhoneNumber>().number(), phoneNumberDetail.number());
    QCOMPARE(entry->customId(), customId);
    QCOMPARE(entry->contact(), contact);
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
    QSignalSpy signalSpy(contactModel, SIGNAL(contactRemoved(QString)));

    QString id("contact1");
    QContact contact;
    ContactCustomId customId;
    // the custom id details holds all the backend IDs, but we are just using the last one
    customId.setCustomId("anotherid:" + id);
    contact.saveDetail(&customId);
    QVERIFY(contactManager->saveContact(&contact));
    QVERIFY(contactManager->removeContact(contact.localId()));

    QCOMPARE(signalSpy.count(), 1);
    QString removedId = signalSpy.at(0).at(0).toString();
    QCOMPARE(removedId, id);
}

void ContactModelTest::testContactLoadedSignal()
{
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
}

void ContactModelTest::testContactSavedSignal()
{
    QSignalSpy signalSpy(contactModel, SIGNAL(contactSaved(QString,QString)));
    QString guid("contactsavedguid");
    QString customId("contactsavedcustomid");

    QContact contact;
    QContactGuid guidDetail;
    guidDetail.setGuid(guid);
    ContactCustomId customIdDetail;
    customIdDetail.setCustomId("anotherid:" + customId);
    QVERIFY(contact.saveDetail(&guidDetail));
    QVERIFY(contact.saveDetail(&customIdDetail));

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
    QString savedCustomId = signalSpy.at(0).at(1).toString();
    QCOMPARE(savedGuid, guid);
    QCOMPARE(savedCustomId, customId);
}

QTEST_MAIN(ContactModelTest)
#include "ContactModelTest.moc"
