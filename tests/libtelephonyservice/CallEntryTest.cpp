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
#include "telepathytest.h"
#include "callentry.h"
#include "callmanager.h"
#include "telepathyhelper.h"
#include "mockcontroller.h"
#include "ofonoaccountentry.h"
#include "accountentryfactory.h"

Q_DECLARE_METATYPE(AccountEntry*)

class CallEntryTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testIsVoicemail_data();
    void testIsVoicemail();

protected Q_SLOTS:
    void onCallChannelAvailable(const Tp::CallChannelPtr &channel);

private:
    Tp::AccountPtr mTpAccount;
    OfonoAccountEntry *mAccount;
    Tp::CallChannelPtr mCallChannel;
    MockController *mMockController;
};

void CallEntryTest::initTestCase()
{
    qRegisterMetaType<AccountEntry*>();
    initialize();
    TelepathyHelper::instance()->registerChannelObserver();
    connect(TelepathyHelper::instance()->channelObserver(),
            SIGNAL(callChannelAvailable(Tp::CallChannelPtr)),
            SLOT(onCallChannelAvailable(Tp::CallChannelPtr)));
}

void CallEntryTest::init()
{
    QSignalSpy accountSpy(TelepathyHelper::instance(), SIGNAL(accountAdded(AccountEntry*)));
    mTpAccount = addAccount("mock", "ofono", "the account");
    QVERIFY(!mTpAccount.isNull());
    TRY_COMPARE(accountSpy.count(), 1);
    mAccount = qobject_cast<OfonoAccountEntry*>(accountSpy.first().first().value<AccountEntry*>());
    QVERIFY(mAccount);
    TRY_VERIFY(mAccount->ready());

    // and create the mock controller
    mMockController = new MockController("ofono", this);
}

void CallEntryTest::cleanup()
{
    doCleanup();

    mMockController->deleteLater();
    mAccount->deleteLater();
    mCallChannel = Tp::CallChannelPtr();

    // wait until all the calls are gone before the next test
    TRY_VERIFY(!CallManager::instance()->hasCalls());
}

void CallEntryTest::testIsVoicemail_data()
{
    QTest::addColumn<QString>("voicemailNumber");
    QTest::addColumn<QString>("callNumber");
    QTest::addColumn<bool>("isVoicemail");

    QTest::newRow("calling voicemail") << "*1234" << "*1234" << true;
    QTest::newRow("calling other number") << "*1234" << "5555555" << false;
}

void CallEntryTest::testIsVoicemail()
{
    QFETCH(QString, voicemailNumber);
    QFETCH(QString, callNumber);
    QFETCH(bool, isVoicemail);

    mMockController->SetVoicemailNumber(voicemailNumber);
    TRY_COMPARE(mAccount->voicemailNumber(), voicemailNumber);

    // now place a call to a number that is not the voicemail number
    QVariantMap properties;
    properties["Caller"] = callNumber;
    properties["State"] = "incoming";
    mMockController->placeCall(properties);
    TRY_VERIFY(!mCallChannel.isNull());
    QCOMPARE(mCallChannel->targetContact()->id(), callNumber);

    CallEntry *callEntry = new CallEntry(mCallChannel);
    QCOMPARE(callEntry->isVoicemail(), isVoicemail);

    callEntry->endCall();
    callEntry->deleteLater();
}

void CallEntryTest::onCallChannelAvailable(const Tp::CallChannelPtr &channel)
{
    mCallChannel = channel;
}

QTEST_MAIN(CallEntryTest)
#include "CallEntryTest.moc"
