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
#include "notificationmenu.h"

class NotificationMenuTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRegularMenu();
    void testResponseMenu();
    void testPasswordMenu();

private:
    bool isRegistered(const QString &service);
};



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

    QDBusPendingReply<QDBusArgument> menuReply = menuInterface.asyncCall("Start", QVariant::fromValue(QList<uint>() << 0));
    QTRY_VERIFY(menuReply.isFinished());
    QVERIFY(menuReply.isValid());

    // parse the reply
    const QDBusArgument argument(menuReply.value());

    argument.beginArray();
    while (!argument.atEnd()) {
        uint group, number;
        argument >> group;
        argument >> number;
        argument.beginArray();
        QList<QVariantMap> entries;
        while (!argument.atEnd()) {
            QVariantMap entry;
            argument >> entry;
            entries << entry;
        }
        argument.endArray();
    }
    argument.endArray();


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
