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

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include "telepathytest.h"
#include "accountentry.h"
#include "chatmanager.h"
#include "telepathyhelper.h"
#include "mockcontroller.h"

class ChatEntryTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testContactChatState_data();
    void testContactChatState();

private:
    Tp::AccountPtr mGenericTpAccount;
    MockController *mGenericMockController;
    Tp::AccountPtr mMultimediaTpAccount;
    MockController *mMultimediaMockController;
};

void ChatEntryTest::initTestCase()
{
    initialize();

    TelepathyHelper::instance()->registerChannelObserver();

    // just give telepathy some time to register the observer
    QTest::qWait(1000);
}

void ChatEntryTest::init()
{
    // add two accounts
    mGenericTpAccount = addAccount("mock", "mock", "the generic account");
    QTRY_VERIFY(!mGenericTpAccount->connection().isNull());

    mMultimediaTpAccount = addAccount("mock", "multimedia", "the multimedia account");
    QTRY_VERIFY(!mMultimediaTpAccount->connection().isNull());

    // and create the mock controller
    mGenericMockController = new MockController("mock", this);

    // and create the mock controller
    mMultimediaMockController = new MockController("multimedia", this);
}

void ChatEntryTest::cleanup()
{
    // the accounts are removed in the parent class.
    doCleanup();

    mGenericMockController->deleteLater();
    mMultimediaMockController->deleteLater();
}

void ChatEntryTest::testContactChatState_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QStringList>("participants");

    QTest::newRow("chat state on generic account") << QString("mock/mock/account0") << (QStringList() << "user@domain.com");
    QTest::newRow("group chat state on generic account") << QString("mock/mock/account0") << (QStringList() << "user@domain.com" << "user2@domain.com");
    QTest::newRow("chat state on multimedia account") << QString("mock/multimedia/account0") << (QStringList() << "+1234567");
    QTest::newRow("group state on multimedia account") << QString("mock/multimedia/account0") << (QStringList() << "+1234567" << "+111111");
}

void ChatEntryTest::testContactChatState()
{
    QFETCH(QString, accountId);
    QFETCH(QStringList, participants);

    MockController *mockController = accountId.startsWith("mock/mock") ? mGenericMockController : mMultimediaMockController;

    QSignalSpy chatEntryCreatedSpy(ChatManager::instance(), SIGNAL(chatEntryCreated(QString, QStringList,ChatEntry *)));
    QVariantMap properties;
    properties["Participants"] = participants;
    ChatEntry *entry = ChatManager::instance()->chatEntryForProperties(accountId, properties, true);
    QVERIFY(entry == NULL);
    QTRY_COMPARE(chatEntryCreatedSpy.count(), 1);

    entry = ChatManager::instance()->chatEntryForProperties(accountId, properties, false);
    QVERIFY(entry != NULL);
    QList<QVariant> arguments = chatEntryCreatedSpy.takeFirst();
    QCOMPARE(accountId, arguments.at(0).toString());
    QCOMPARE(participants.toSet(), arguments.at(1).toStringList().toSet());
    QCOMPARE(entry, arguments.at(2).value<ChatEntry*>());
    
    QQmlListProperty<ContactChatState> chatStates = entry->chatStates();
    QCOMPARE(entry->chatStatesCount(&chatStates), participants.count());

    // change state of contact 1
    ContactChatState *contactChatState1 = entry->chatStatesAt(&chatStates, 0);
    QSignalSpy chatStateChangedSpy1(contactChatState1, SIGNAL(stateChanged()));
    mockController->ChangeChatState(participants, contactChatState1->contactId(), ChatEntry::ChannelChatStateComposing);
    QTRY_COMPARE(chatStateChangedSpy1.count(), 1);
    QCOMPARE(contactChatState1->state(), (int)ChatEntry::ChannelChatStateComposing);
    chatStateChangedSpy1.clear();

    mockController->ChangeChatState(participants, contactChatState1->contactId(), ChatEntry::ChannelChatStatePaused);
    QTRY_COMPARE(chatStateChangedSpy1.count(), 1);
    QCOMPARE(entry->chatStatesAt(&chatStates, 0)->state(), (int)ChatEntry::ChannelChatStatePaused);

    if (participants.count() > 1) {
        // change state of contact 2
        ContactChatState *contactChatState2 = entry->chatStatesAt(&chatStates, 1);
        QSignalSpy chatStateChangedSpy2(contactChatState2, SIGNAL(stateChanged()));
        mockController->ChangeChatState(participants, contactChatState2->contactId(), ChatEntry::ChannelChatStateComposing);
        QTRY_COMPARE(chatStateChangedSpy2.count(), 1);
        QCOMPARE(contactChatState2->state(), (int)ChatEntry::ChannelChatStateComposing);
        chatStateChangedSpy2.clear();

        mockController->ChangeChatState(participants, contactChatState2->contactId(), ChatEntry::ChannelChatStatePaused);
        QTRY_COMPARE(chatStateChangedSpy2.count(), 1);
        QCOMPARE(contactChatState2->state(), (int)ChatEntry::ChannelChatStatePaused);
    }
}

QTEST_MAIN(ChatEntryTest)
#include "ChatEntryTest.moc"
