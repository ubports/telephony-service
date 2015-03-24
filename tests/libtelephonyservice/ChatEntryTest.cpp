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
    void testContactChatState();

private:
    Tp::AccountPtr mGenericTpAccount;
    MockController *mGenericMockController;
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

    // and create the mock controller
    mGenericMockController = new MockController("mock", this);
}

void ChatEntryTest::cleanup()
{
    // the accounts are removed in the parent class.
    doCleanup();

    mGenericMockController->deleteLater();
}

void ChatEntryTest::testContactChatState()
{
    QStringList recipients;
    recipients << "user@domain.com" << "user2@domain.com";
    QSignalSpy chatEntryCreatedSpy(ChatManager::instance(), SIGNAL(chatEntryCreated(QString, QStringList,ChatEntry *)));
    ChatEntry *entry = ChatManager::instance()->chatEntryForParticipants("mock/mock/account0", recipients, true);
    QVERIFY(entry == NULL);
    QTRY_COMPARE(chatEntryCreatedSpy.count(), 1);

    entry = ChatManager::instance()->chatEntryForParticipants("mock/mock/account0", recipients, false);
    QVERIFY(entry != NULL);
    QList<QVariant> arguments = chatEntryCreatedSpy.takeFirst();
    QCOMPARE(QString("mock/mock/account0"), arguments.at(0).toString());
    QCOMPARE(recipients, arguments.at(1).toStringList());
    QCOMPARE(entry, arguments.at(2).value<ChatEntry*>());
    
    QQmlListProperty<ContactChatState> chatStates = entry->chatStates();
    QCOMPARE(entry->chatStatesCount(&chatStates), 1);

    QSignalSpy chatStateChangedSpy(entry->chatStatesAt(&chatStates, 0), SIGNAL(stateChanged()));
    mGenericMockController->changeChatState("user@domain.com", ChatEntry::ChannelChatStateComposing);
    QTRY_COMPARE(chatStateChangedSpy.count(), 1);
    QCOMPARE(entry->chatStatesAt(&chatStates, 0)->state(), (int)ChatEntry::ChannelChatStateComposing);
    chatStateChangedSpy.clear();

    mGenericMockController->changeChatState("user@domain.com", ChatEntry::ChannelChatStatePaused);
    QTRY_COMPARE(chatStateChangedSpy.count(), 1);
    QCOMPARE(entry->chatStatesAt(&chatStates, 0)->state(), (int)ChatEntry::ChannelChatStatePaused);
    chatStateChangedSpy.clear();

    mGenericMockController->changeChatState("user2@domain.com", ChatEntry::ChannelChatStateComposing);
    QTRY_COMPARE(chatStateChangedSpy.count(), 1);
    QCOMPARE(entry->chatStatesAt(&chatStates, 0)->state(), (int)ChatEntry::ChannelChatStateComposing);
    chatStateChangedSpy.clear();

    mGenericMockController->changeChatState("user2@domain.com", ChatEntry::ChannelChatStatePaused);
    QTRY_COMPARE(chatStateChangedSpy.count(), 1);
    QCOMPARE(entry->chatStatesAt(&chatStates, 0)->state(), (int)ChatEntry::ChannelChatStatePaused);
}

QTEST_MAIN(ChatEntryTest)
#include "ChatEntryTest.moc"
