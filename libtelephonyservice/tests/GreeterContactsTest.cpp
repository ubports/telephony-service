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

#include "greetercontacts.h"

#include <QContact>
#include <QContactAvatar>
#include <QContactName>
#include <QContactPhoneNumber>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusReply>
#include <QObject>
#include <QtTest>
#include <QUrl>

Q_DECLARE_METATYPE(QtContacts::QContact) // for QVariant's benefit

QTCONTACTS_USE_NAMESPACE

class GreeterContactsTest : public QObject
{
    Q_OBJECT

public:
    GreeterContactsTest();

public Q_SLOTS:
    void setFilter();

private Q_SLOTS:
    void initTestCase();
    void cleanup();
    void testContactToMap();
    void testMapToContact();
    void testInitialValues();
    void testSignalOnFilter();
    void testSignalOnEntry();
    void testSignalOnEntryInvalidated();
    void testSignalOnContacts();
    void testSignalOnContactsInvalidated();
    void testEmitContact();

private:
    void waitForUpdatedSignal(bool convertedPath = false);
    void makeGreeterContacts();
    void waitForInitialQuery();
    QContact makeTestContact(bool convertedPath = false);
    QVariantMap makeTestMap();
    void setActiveEntry(const QString &entry);
    void setCurrentContact(const QVariantMap &map);
    void setUseInvalidated(const QString &path, const QString &interface, bool useInvalidated);

    GreeterContacts *mGreeterContacts;
    QSignalSpy *mSpy;
};



GreeterContactsTest::GreeterContactsTest()
: QObject()
{
}

void GreeterContactsTest::initTestCase()
{
    mGreeterContacts = NULL;
    mSpy = NULL;
    qRegisterMetaType<QContact>();
}

void GreeterContactsTest::cleanup()
{
    if (mSpy) {
        delete mSpy;
        mSpy = NULL;
    }
    if (mGreeterContacts) {
        delete mGreeterContacts;
        mGreeterContacts = NULL;
    }

    setActiveEntry("");
    setCurrentContact(QVariantMap());
    setUseInvalidated("/list", "com.canonical.UnityGreeter.List", false);
    setUseInvalidated("/org/freedesktop/Accounts/User12345", "com.canonical.TelephonyServiceApprover", false);
}

void GreeterContactsTest::testContactToMap()
{
    QVariantMap map = GreeterContacts::contactToMap(makeTestContact());
    QVariantMap expectedMap = makeTestMap();
    QCOMPARE(map, expectedMap);
}

void GreeterContactsTest::testMapToContact()
{
    QContact contact = GreeterContacts::mapToContact(makeTestMap());
    QContact expectedContact = makeTestContact();
    QCOMPARE(contact, expectedContact);
}

void GreeterContactsTest::testInitialValues()
{
    setActiveEntry("testuser");
    setCurrentContact(makeTestMap());
    makeGreeterContacts();
    setFilter();
    waitForUpdatedSignal();
}

void GreeterContactsTest::testSignalOnFilter()
{
    setActiveEntry("testuser");
    setCurrentContact(makeTestMap());
    makeGreeterContacts();
    waitForInitialQuery();
    // setFilter might immediately send the signal, so wait until we can start the spy
    QTimer::singleShot(0, this, SLOT(setFilter()));
    waitForUpdatedSignal();
}

void GreeterContactsTest::testSignalOnEntry()
{
    setCurrentContact(makeTestMap());
    makeGreeterContacts();
    setFilter();
    waitForInitialQuery();
    setActiveEntry("testuser");
    waitForUpdatedSignal();
}

void GreeterContactsTest::testSignalOnEntryInvalidated()
{
    setCurrentContact(makeTestMap());
    makeGreeterContacts();
    setFilter();
    waitForInitialQuery();
    setUseInvalidated("/list", "com.canonical.UnityGreeter.List", true);
    setActiveEntry("testuser");
    waitForUpdatedSignal();
}

void GreeterContactsTest::testSignalOnContacts()
{
    setActiveEntry("testuser");
    makeGreeterContacts();
    setFilter();
    waitForInitialQuery();
    setCurrentContact(makeTestMap());
    waitForUpdatedSignal();
}

void GreeterContactsTest::testSignalOnContactsInvalidated()
{
    setActiveEntry("testuser");
    makeGreeterContacts();
    setFilter();
    waitForInitialQuery();
    setUseInvalidated("/org/freedesktop/Accounts/User12345", "com.canonical.TelephonyServiceApprover", true);
    setCurrentContact(makeTestMap());
    waitForUpdatedSignal();
}

void GreeterContactsTest::testEmitContact()
{
    setActiveEntry("testuser");
    makeGreeterContacts();
    setFilter();
    waitForInitialQuery();
    // this next line acts like setCurrentContact() because uid is set to 12345
    GreeterContacts::emitContact(makeTestContact());
    waitForUpdatedSignal(true);
}

QVariantMap GreeterContactsTest::makeTestMap()
{
    QVariantMap map;
    map.insert("FirstName", QVariant("First"));
    map.insert("Image", QVariant(CMAKE_SOURCE_DIR "/icons/hicolor/48x48/apps/telephony-service-call.png"));
    map.insert("LastName", QVariant("Last"));
    map.insert("PhoneNumber", QVariant("555"));
    return map;
}

QContact GreeterContactsTest::makeTestContact(bool convertedPath)
{
    // This is the same contact that the test server will give out.
    QContact contact;

    QContactAvatar avatarDetail;
    QString imagePath;
    if (convertedPath) {
        imagePath = qgetenv("XDG_GREETER_DATA_DIR") + "/telephony-service/contact-image";
    } else {
        imagePath = CMAKE_SOURCE_DIR "/icons/hicolor/48x48/apps/telephony-service-call.png";
    }
    avatarDetail.setValue(QContactAvatar::FieldImageUrl, QUrl::fromLocalFile(imagePath));
    contact.saveDetail(&avatarDetail);

    QContactName nameDetail;
    nameDetail.setValue(QContactName::FieldFirstName, "First");
    nameDetail.setValue(QContactName::FieldLastName, "Last");
    contact.saveDetail(&nameDetail);

    QContactPhoneNumber numberDetail;
    numberDetail.setValue(QContactPhoneNumber::FieldNumber, "555");
    contact.saveDetail(&numberDetail);

    return contact;
}

void GreeterContactsTest::setFilter()
{
    mGreeterContacts->setFilter(QContactPhoneNumber::match("555"));
}

void GreeterContactsTest::setActiveEntry(const QString &entry)
{
    QDBusInterface iface("com.canonical.UnityGreeter",
                         "/list",
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::sessionBus());
    QDBusReply<void> reply = iface.call("Set", "com.canonical.UnityGreeter.List", "ActiveEntry", QVariant::fromValue(QDBusVariant(QVariant(entry))));
    QVERIFY(reply.isValid());
}

void GreeterContactsTest::setCurrentContact(const QVariantMap &map)
{
    QDBusInterface iface("org.freedesktop.Accounts",
                         "/org/freedesktop/Accounts/User12345",
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::sessionBus());
    QDBusReply<void> reply = iface.call("Set", "com.canonical.TelephonyServiceApprover", "CurrentContact", QVariant::fromValue(QDBusVariant(QVariant(map))));
    QVERIFY(reply.isValid());
}

void GreeterContactsTest::waitForInitialQuery()
{
    QDBusInterface iface("org.freedesktop.Accounts",
                         "/org/freedesktop/Accounts/User12345",
                         "com.canonical.TelephonyServiceApprover",
                         QDBusConnection::sessionBus());
    QSignalSpy spy(&iface, SIGNAL(InitialQueriesDone()));
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
}

void GreeterContactsTest::makeGreeterContacts()
{
    mGreeterContacts = new GreeterContacts();
    mSpy = new QSignalSpy(mGreeterContacts, SIGNAL(contactUpdated(QtContacts::QContact)));
}

void GreeterContactsTest::waitForUpdatedSignal(bool convertedPath)
{
    QVERIFY(mSpy->wait());
    QCOMPARE(mSpy->count(), 1);

    QList<QVariant> arguments = mSpy->takeFirst();
    QContact expectedContact = makeTestContact(convertedPath);
    QCOMPARE(arguments.at(0).value<QContact>(), expectedContact);
}

void GreeterContactsTest::setUseInvalidated(const QString &path, const QString &interface, bool useInvalidated)
{
    QDBusInterface iface("org.freedesktop.Accounts",
                         path,
                         interface,
                         QDBusConnection::sessionBus());
    QDBusReply<void> reply = iface.call("SetUseInvalidated", useInvalidated);
    QVERIFY(reply.isValid());
}

QTEST_MAIN(GreeterContactsTest)
#include "GreeterContactsTest.moc"
