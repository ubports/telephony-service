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

#include "config.h"
#include "protocolmanager.h"

Q_DECLARE_METATYPE(Protocols)
Q_DECLARE_METATYPE(Protocol::Features)
class ProtocolManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testNumberOfProtocols();
    void testFooProtocolLoaded();
    void testProtocolNames();
    void testProtocolByName();
    void testProtocolsForFeatures_data();
    void testProtocolsForFeatures();
    void testTextProtocols();
    void testVoiceProtocols();
    void testIsProtocolSupported_data();
    void testIsProtocolSupported();
};

void ProtocolManagerTest::initTestCase()
{
    qRegisterMetaType<Protocols>();
    qRegisterMetaType<Protocol::Features>();
}

void ProtocolManagerTest::testNumberOfProtocols()
{
    QDir dir(protocolsDir());
    int count = dir.entryList(QDir::Files).count();
    QCOMPARE(ProtocolManager::instance()->protocols().count(), count);
}

void ProtocolManagerTest::testFooProtocolLoaded()
{
    Protocol *fooProtocol = 0;
    Q_FOREACH(Protocol *proto, ProtocolManager::instance()->protocols()) {
        QVERIFY(proto);
        if (proto->name() == "foo") {
            fooProtocol = proto;
            break;
        }
    }
    QVERIFY(fooProtocol);
}

void ProtocolManagerTest::testProtocolNames()
{
    Protocols protocols = ProtocolManager::instance()->protocols();
    QStringList protocolNames = ProtocolManager::instance()->protocolNames();
    for (int i = 0; i < protocols.count(); ++i) {
        QCOMPARE(protocols[i]->name(), protocolNames[i]);
    }
}

void ProtocolManagerTest::testProtocolByName()
{
    Protocol *invalid = ProtocolManager::instance()->protocolByName("invalid");
    QVERIFY(!invalid);

    Protocol *bar = ProtocolManager::instance()->protocolByName("bar");
    QVERIFY(bar);
    QCOMPARE(bar->name(), QString("bar"));
}

void ProtocolManagerTest::testProtocolsForFeatures_data()
{
    QTest::addColumn<Protocol::Features>("features");
    QTest::addColumn<Protocols>("protocols");

    Protocols protocols;
    protocols = ProtocolManager::instance()->protocols();
    QTest::newRow("both text and voice") << Protocol::Features(Protocol::TextChats | Protocol::VoiceCalls) << protocols;

    protocols.clear();
    protocols << ProtocolManager::instance()->protocolByName("foo")
              << ProtocolManager::instance()->protocolByName("bar")
              << ProtocolManager::instance()->protocolByName("text");
    QTest::newRow("only text") << Protocol::Features(Protocol::TextChats) << protocols;

    protocols.clear();
    protocols << ProtocolManager::instance()->protocolByName("foo")
              << ProtocolManager::instance()->protocolByName("bar")
              << ProtocolManager::instance()->protocolByName("voice");
    QTest::newRow("only voice") << Protocol::Features(Protocol::VoiceCalls) << protocols;

    QTest::newRow("no features") << Protocol::Features() << Protocols();
}

void ProtocolManagerTest::testProtocolsForFeatures()
{
    QFETCH(Protocol::Features, features);
    QFETCH(Protocols, protocols);

    Protocols filtered = ProtocolManager::instance()->protocolsForFeatures(features);
    QCOMPARE(filtered.count(), protocols.count());
    Q_FOREACH(Protocol *protocol, protocols) {
        QVERIFY(filtered.contains(protocol));
    }
}

void ProtocolManagerTest::testTextProtocols()
{
    QCOMPARE(ProtocolManager::instance()->textProtocols(), ProtocolManager::instance()->protocolsForFeatures(Protocol::TextChats));
}

void ProtocolManagerTest::testVoiceProtocols()
{
    QCOMPARE(ProtocolManager::instance()->voiceProtocols(), ProtocolManager::instance()->protocolsForFeatures(Protocol::VoiceCalls));
}

void ProtocolManagerTest::testIsProtocolSupported_data()
{
    QTest::addColumn<QString>("protocolName");
    QTest::addColumn<bool>("supported");

    QTest::newRow("foo") << QString("foo") << true;
    QTest::newRow("bar") << QString("bar") << true;
    QTest::newRow("text") << QString("text") << true;
    QTest::newRow("voice") << QString("voice") << true;
    QTest::newRow("invalid") << QString("invalid") << false;
}

void ProtocolManagerTest::testIsProtocolSupported()
{
    QFETCH(QString, protocolName);
    QFETCH(bool, supported);
    QCOMPARE(ProtocolManager::instance()->isProtocolSupported(protocolName), supported);
}

QTEST_MAIN(ProtocolManagerTest)
#include "ProtocolManagerTest.moc"
