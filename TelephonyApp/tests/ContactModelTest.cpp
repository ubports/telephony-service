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

using namespace QtMobility;

class ContactModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testRowCount();

private:
    QContactManager *contactManager;
};

void ContactModelTest::initTestCase()
{
    contactManager = new QContactManager("memory");
    ContactModel::instance()->setContactManager(contactManager);
}

void ContactModelTest::cleanupTestCase()
{
    delete contactManager;
}

void ContactModelTest::testRowCount()
{
    ContactModel *model = ContactModel::instance();
    int rowCount = model->rowCount();

    // add contact
    QContact contact;
    contactManager->saveContact(&contact);
    QCOMPARE(model->rowCount(), rowCount+1);

    // then remove the contact
    contactManager->removeContact(contact.localId());
    QCOMPARE(model->rowCount(), rowCount);
}

QTEST_MAIN(ContactModelTest)
#include "ContactModelTest.moc"
