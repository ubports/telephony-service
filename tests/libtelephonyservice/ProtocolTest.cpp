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
#include "protocol.h"

// just to make the constructor public
class TestProtocol : public Protocol
{
    Q_OBJECT
public:
    TestProtocol(const QString &name, Protocol::Features features, const QString &fallbackProtocol, const QString &backgroundFile, const QString &icon, const QString &serviceName = QString::null, QObject *parent = 0)
     : Protocol(name, features, fallbackProtocol, backgroundFile, icon, serviceName, parent) { }
};

class ProtocolTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testBasicInfo();
    void testFromFile();
};

void ProtocolTest::testBasicInfo()
{
    QString name("foobar");
    Protocol::Features features(Protocol::TextChats);
    QString fallbackProtocol("theFallback");
    QString backgroundImage("/tmp/background.png");
    QString icon("/tmp/icon.png");
    QString serviceName("The service");

    TestProtocol protocol(name, features, fallbackProtocol, backgroundImage, icon, serviceName, this);
    QCOMPARE(protocol.name(), name);
    QCOMPARE(protocol.features(), features);
    QCOMPARE(protocol.fallbackProtocol(), fallbackProtocol);
    QCOMPARE(protocol.backgroundImage(), backgroundImage);
    QCOMPARE(protocol.icon(), icon);
    QCOMPARE(protocol.serviceName(), serviceName);
    QCOMPARE(protocol.parent(), this);
}

void ProtocolTest::testFromFile()
{
    // check that calling fromFile() on an invalid path returns 0
    Protocol *nullProtocol = Protocol::fromFile("/non/existent/path/to/a/dummy.protocol");
    QVERIFY(!nullProtocol);

    // and now check a valid protocol
    Protocol *protocol = Protocol::fromFile(protocolsDir() + "/foo.protocol");
    QVERIFY(protocol);
    QCOMPARE(protocol->name(), QString("foo"));
    QCOMPARE(protocol->features(), Protocol::Features(Protocol::TextChats | Protocol::VoiceCalls));
    QCOMPARE(protocol->fallbackProtocol(), QString("bar"));
    QCOMPARE(protocol->backgroundImage(), QString("/tmp/background.png"));
    QCOMPARE(protocol->icon(), QString("/tmp/icon.png"));
    QCOMPARE(protocol->serviceName(), QString("The Service"));
}

QTEST_MAIN(ProtocolTest)
#include "ProtocolTest.moc"
