/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include "conversationfeeditem.h"

class ConversationFeedItemTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testContactId();
    void testContactAlias();
    void testContactAvatar();
    void testIncoming();
    void testTimestamp();

private:
    ConversationFeedItem *item;
};

void ConversationFeedItemTest::initTestCase()
{
    item = new ConversationFeedItem(this);
}

void ConversationFeedItemTest::cleanupTestCase()
{
    delete item;
}

void ConversationFeedItemTest::testContactId()
{
    QSignalSpy signalSpy(item, SIGNAL(contactIdChanged()));
    QString id("OneContactId");
    item->setContactId(id);

    QCOMPARE(item->contactId(), id);
    QCOMPARE(signalSpy.count(), 1);
}

void ConversationFeedItemTest::testContactAlias()
{
    QSignalSpy signalSpy(item, SIGNAL(contactAliasChanged()));
    QString alias("OneContactAlias");
    item->setContactAlias(alias);

    QCOMPARE(item->contactAlias(), alias);
    QCOMPARE(signalSpy.count(), 1);
}

void ConversationFeedItemTest::testContactAvatar()
{
    QSignalSpy signalSpy(item, SIGNAL(contactAvatarChanged()));
    QUrl avatar("/a/file/somewhere");
    item->setContactAvatar(avatar);

    QCOMPARE(item->contactAvatar(), avatar);
    QCOMPARE(signalSpy.count(), 1);
}

void ConversationFeedItemTest::testIncoming()
{
    QSignalSpy signalSpy(item, SIGNAL(incomingChanged()));
    bool incoming = false;
    item->setIncoming(incoming);

    QCOMPARE(item->incoming(), incoming);
    QCOMPARE(signalSpy.count(), 1);
}

void ConversationFeedItemTest::testTimestamp()
{
    QSignalSpy signalSpy(item, SIGNAL(timestampChanged()));
    QDateTime timestamp = QDateTime::currentDateTime();
    item->setTimestamp(timestamp);

    QCOMPARE(item->timestamp(), timestamp);
    QCOMPARE(signalSpy.count(), 1);
}

QTEST_MAIN(ConversationFeedItemTest)
#include "ConversationFeedItemTest.moc"
