/*
 * Copyright (C) 2015 Canonical, Ltd.
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
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QDBusMetaType>
#include "notificationmenu.h"

class NotificationMenuTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testRegularMenu();
    void testResponseMenu();
    void testPasswordMenu();

private:
    bool isRegistered(const QString &service);
};

typedef struct {
    uint group;
    uint number;
    QList<QVariantMap> entries;
} Menu;

Q_DECLARE_METATYPE(QList<Menu>)

QDBusArgument &operator<<(QDBusArgument &argument, const QList<Menu> &menuList)
{
    qDebug() << "BLABLA1";
    argument.beginArray();
    Q_FOREACH(const Menu &menu, menuList) {
        qDebug() << "BLABLA2";
        argument.beginStructure();
        argument << menu.group;
        argument << menu.number;
        qDebug() << "BLABLA3";
        argument.beginArray();
        Q_FOREACH(const QVariantMap &entry, menu.entries) {
            argument << entry;
        }
        qDebug() << "BLABLA4";
        argument.endArray();
        qDebug() << "BLABLA5";
        argument.endStructure();
        qDebug() << "BLABLA6";
    }
    qDebug() << "BLABLA7";
    argument.endArray();
    qDebug() << "BLABLA8";
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QList<Menu> &menuList)
{
    qDebug() << ">>Argument type:" << argument.currentSignature() << argument.currentType();
    argument.beginArray();
    while (!argument.atEnd()) {
        argument.beginStructure();
        qDebug() << "Argument type:" << argument.currentSignature() << argument.currentType();
        Menu menu;
        argument >> menu.group;
        argument >> menu.number;
        argument.beginArray();
        while (!argument.atEnd()) {
            QVariantMap entry;
            argument >> entry;
            menu.entries << entry;
        }
        argument.endArray();
        argument.endStructure();
        menuList << menu;
    }
    argument.endArray();
    return argument;
}

void NotificationMenuTest::initTestCase()
{
    qRegisterMetaType<QList<Menu>>();
    qDBusRegisterMetaType<QList<Menu>>();
}

void NotificationMenuTest::testRegularMenu()
{
    QString id("regularmenu");
    NotificationMenu menu(id, false, false);
    QVERIFY(isRegistered(menu.busName()));
    QDBusInterface menuInterface(menu.busName(), menu.menuPath(), "org.gtk.Menus");
    QVERIFY(menuInterface.isValid());
    // FIXME: check what else to verify in the menuInterface, for now checking that it is there is enough
    // as this is created by libgmenumodel

    QDBusInterface actionInterface(menu.busName(), menu.actionPath(), "org.gtk.Actions");
    QVERIFY(actionInterface.isValid());

    QDBusPendingReply<QStringList> listReply = actionInterface.asyncCall("List");
    QTRY_VERIFY(listReply.isFinished());
    QVERIFY(listReply.isValid());

    // check that the actions contains the required id
    QStringList actionsList = listReply.value();
    QCOMPARE(actionsList.count(), 1);
    QCOMPARE(actionsList.first(), id);
}

void NotificationMenuTest::testResponseMenu()
{
    QString id("responsemenu");
    NotificationMenu menu(id, true, false);
    QVERIFY(isRegistered(menu.busName()));
    QDBusInterface menuInterface(menu.busName(), menu.menuPath(), "org.gtk.Menus");
    QVERIFY(menuInterface.isValid());
    qDebug() << "BLABLA going to call";

    QDBusPendingReply<QList<Menu>> menuReply = menuInterface.asyncCall("Start", QVariant::fromValue(QList<uint>() << 0));
    QTRY_VERIFY(menuReply.isFinished());
    QVERIFY(menuReply.isValid());

    qDebug() << "BLABLA going to get the reply";
    // parse the reply
    QList<Menu> menus = menuReply.value();
    qDebug()<< "Got reply.";

    qDebug() << menus.count();

    QDBusInterface actionInterface(menu.busName(), menu.actionPath(), "org.gtk.Actions");
    QVERIFY(actionInterface.isValid());

    QDBusPendingReply<QStringList> listReply = actionInterface.asyncCall("List");
    QTRY_VERIFY(listReply.isFinished());
    QVERIFY(listReply.isValid());

    // check that the actions contains the required id
    QStringList actionsList = listReply.value();
    QCOMPARE(actionsList.count(), 1);
    QCOMPARE(actionsList.first(), id);
}

void NotificationMenuTest::testPasswordMenu()
{

}

bool NotificationMenuTest::isRegistered(const QString &service)
{
    bool result = false;
    QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered(service);
    if (reply.isValid()) {
        result = reply.value();
    }
    return result;
}

QTEST_MAIN(NotificationMenuTest)
#include "NotificationMenuTest.moc"
