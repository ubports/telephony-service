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

#include "greetercontacts.h"

#include <pwd.h>
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
#include <unistd.h>

Q_DECLARE_METATYPE(QtContacts::QContact) // for QVariant's benefit

QTCONTACTS_USE_NAMESPACE

class GreeterContactsWrapper : public GreeterContacts
{
    Q_OBJECT
public:
    GreeterContactsWrapper() : GreeterContacts() {}
};

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
    void testGreeterIsActive();

private:
    void waitForUpdatedSignal(bool convertedPath = false);
    void makeGreeterContacts();
    void waitForInitialQuery();
    QContact makeTestContact(bool convertedPath = false);
    QVariantMap makeTestMap();
    void setActiveEntry(bool currentUser);
    void setCurrentContact(const QVariantMap &map);
    void setUseInvalidated(const QString &path, const QString &interface, bool useInvalidated);
    void setGreeterActive(bool active);

    QString mUserPath;
    GreeterContacts *mGreeterContacts;
    QSignalSpy *mSpy;
};



GreeterContactsTest::GreeterContactsTest()
: QObject()
{
    mUserPath = "/org/freedesktop/Accounts/User" + QString::number(getuid());
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

    setActiveEntry(false);
    setCurrentContact(QVariantMap());
    setUseInvalidated("/list", "com.canonical.UnityGreeter.List", false);
    setUseInvalidated(mUserPath, "com.canonical.TelephonyServiceApprover", false);
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
    setActiveEntry(true);
    setCurrentContact(makeTestMap());
    makeGreeterContacts();
    setFilter();
    waitForUpdatedSignal();
}

void GreeterContactsTest::testSignalOnFilter()
{
    setActiveEntry(true);
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
    setActiveEntry(true);
    waitForUpdatedSignal();
}

void GreeterContactsTest::testSignalOnEntryInvalidated()
{
    setCurrentContact(makeTestMap());
    makeGreeterContacts();
    setFilter();
    waitForInitialQuery();
    setUseInvalidated("/list", "com.canonical.UnityGreeter.List", true);
    setActiveEntry(true);
    waitForUpdatedSignal();
}

void GreeterContactsTest::testSignalOnContacts()
{
    setActiveEntry(true);
    makeGreeterContacts();
    setFilter();
    waitForInitialQuery();
    setCurrentContact(makeTestMap());
    waitForUpdatedSignal();
}

void GreeterContactsTest::testSignalOnContactsInvalidated()
{
    setActiveEntry(true);
    makeGreeterContacts();
    setFilter();
    waitForInitialQuery();
    setUseInvalidated(mUserPath, "com.canonical.TelephonyServiceApprover", true);
    setCurrentContact(makeTestMap());
    waitForUpdatedSignal();
}

void GreeterContactsTest::testEmitContact()
{
    setActiveEntry(true);
    makeGreeterContacts();
    setFilter();
    waitForInitialQuery();
    // this next line acts like setCurrentContact() because uid is set to 12345
    GreeterContacts::emitContact(makeTestContact());
    waitForUpdatedSignal(true);
}

void GreeterContactsTest::testGreeterIsActive()
{
    makeGreeterContacts();

    // check that by default it is false
    QCOMPARE(mGreeterContacts->greeterActive(), false);

    QSignalSpy spy(mGreeterContacts, SIGNAL(greeterActiveChanged()));
    // now set it to true
    setGreeterActive(true);
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(mGreeterContacts->greeterActive(), true);
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
    mGreeterContacts->setContactFilter(QContactPhoneNumber::match("555"));
}

void GreeterContactsTest::setActiveEntry(bool currentUser)
{
    QString entry = "";
    if (currentUser)
        entry = getpwuid(getuid())->pw_name;

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
                         mUserPath,
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::sessionBus());
    QDBusReply<void> reply = iface.call("Set", "com.canonical.TelephonyServiceApprover", "CurrentContact", QVariant::fromValue(QDBusVariant(QVariant(map))));
    QVERIFY(reply.isValid());
}

void GreeterContactsTest::waitForInitialQuery()
{
    QDBusInterface iface("org.freedesktop.Accounts",
                         mUserPath,
                         "com.canonical.TelephonyServiceApprover",
                         QDBusConnection::sessionBus());
    QSignalSpy spy(&iface, SIGNAL(InitialQueriesDone()));
    QTRY_COMPARE(spy.count(), 1);
}

void GreeterContactsTest::makeGreeterContacts()
{
    mGreeterContacts = new GreeterContactsWrapper();
    mSpy = new QSignalSpy(mGreeterContacts, SIGNAL(contactUpdated(QtContacts::QContact)));
}

void GreeterContactsTest::waitForUpdatedSignal(bool convertedPath)
{
    QTRY_COMPARE(mSpy->count(), 1);

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

void GreeterContactsTest::setGreeterActive(bool active)
{
    QDBusInterface iface("com.canonical.UnityGreeter",
                         "/",
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::sessionBus());
    QDBusReply<void> reply = iface.call("Set", "com.canonical.UnityGreeter", "IsActive", QVariant::fromValue(QDBusVariant(QVariant(active))));
    QVERIFY(reply.isValid());
}

QTEST_MAIN(GreeterContactsTest)
#include "GreeterContactsTest.moc"
