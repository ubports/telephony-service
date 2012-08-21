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
#include <QContactName>
#include "contactmodel.h"
#include "contactproxymodel.h"
#include "contactentry.h"

using namespace QtMobility;

Q_DECLARE_METATYPE(ContactEntry)
Q_DECLARE_METATYPE(ContactEntry*)

class ContactProxyModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testFilterChanged();
    void testRowCountNoFilter();
    void testRowCountWithFilter();
    void testData();
    void testDataSorted();

private:
    QContactManager *contactManager;
    ContactModel *contactModel;
    ContactProxyModel *contactProxyModel;
    QStringList contacts;
};

void ContactProxyModelTest::initTestCase()
{
    contacts << "Sue" << "Jan" << "Johnny" << "John" << "Álvaro" << "andrew" << "Abel";
    contactModel = ContactModel::instance("memory");
    contactManager = contactModel->contactManager();

    contactProxyModel = new ContactProxyModel();
    contactProxyModel->setModel(contactModel);

    qRegisterMetaType<ContactEntry>();
    qRegisterMetaType<ContactEntry*>();

    // fill model with dummy contacts
    Q_FOREACH(const QString &name, contacts) {
        QContact contact;
        QContactName nameDetail;
        nameDetail.setCustomLabel(name);
        QVERIFY(contact.saveDetail(&nameDetail));
        QVERIFY(contactManager->saveContact(&contact));
    }
}

void ContactProxyModelTest::cleanupTestCase()
{
    delete contactManager;
    delete contactProxyModel;
}

void ContactProxyModelTest::testFilterChanged()
{
    QSignalSpy signalSpy(contactProxyModel, SIGNAL(filterTextChanged()));
    contactProxyModel->setFilterText(QString("FilterString"));
    QCOMPARE(signalSpy.count(), 1);
}

void ContactProxyModelTest::testRowCountNoFilter()
{
    contactProxyModel->setFilterText(QString());
    QCOMPARE(contactProxyModel->rowCount(), contacts.size());
}

void ContactProxyModelTest::testRowCountWithFilter()
{
    contactProxyModel->setFilterText("Jan");
    QCOMPARE(contactProxyModel->rowCount(), 1);

    contactProxyModel->setFilterText("John");
    QCOMPARE(contactProxyModel->rowCount(), 2);
}

void ContactProxyModelTest::testData()
{
    contactProxyModel->setFilterText("Jan");
    ContactEntry *entry = qobject_cast<ContactEntry*>(contactProxyModel->data(contactProxyModel->index(0,0), ContactModel::ContactRole).value<QObject*>());
    QVERIFY(entry);
    QCOMPARE(entry->displayLabel(), QString("Jan"));

    contactProxyModel->setFilterText("John");
    entry = qobject_cast<ContactEntry*>(contactProxyModel->data(contactProxyModel->index(0,0), ContactModel::ContactRole).value<QObject*>());
    QVERIFY(entry);
    QCOMPARE(entry->displayLabel(), QString("John"));

    entry = qobject_cast<ContactEntry*>(contactProxyModel->data(contactProxyModel->index(1,0), ContactModel::ContactRole).value<QObject*>());
    QVERIFY(entry);
    QCOMPARE(entry->displayLabel(), QString("Johnny"));
}

void ContactProxyModelTest::testDataSorted()
{
    contactProxyModel->setFilterText(QString());
    ContactEntry *entry = qobject_cast<ContactEntry*>(contactProxyModel->data(contactProxyModel->index(0,0), ContactModel::ContactRole).value<QObject*>());
    QVERIFY(entry);
    QCOMPARE(entry->displayLabel(), QString("Abel"));

    entry = qobject_cast<ContactEntry*>(contactProxyModel->data(contactProxyModel->index(1,0), ContactModel::ContactRole).value<QObject*>());
    QVERIFY(entry);
    QCOMPARE(entry->displayLabel(), QString("Álvaro"));

    entry = qobject_cast<ContactEntry*>(contactProxyModel->data(contactProxyModel->index(2,0), ContactModel::ContactRole).value<QObject*>());
    QVERIFY(entry);
    QCOMPARE(entry->displayLabel(), QString("andrew"));
}

QTEST_MAIN(ContactProxyModelTest)
#include "ContactProxyModelTest.moc"
