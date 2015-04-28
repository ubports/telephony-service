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
#include "telepathyhelper.h"
#include "mockcontroller.h"
#include "ofonoaccountentry.h"
#include "accountentryfactory.h"

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
    initialize();
    TelepathyHelper::instance()->registerChannelObserver();
    connect(TelepathyHelper::instance()->channelObserver(),
            SIGNAL(callChannelAvailable(Tp::CallChannelPtr)),
            SLOT(onCallChannelAvailable(Tp::CallChannelPtr)));
}

void CallEntryTest::init()
{
    mTpAccount = addAccount("mock", "ofono", "the account");
    QVERIFY(!mTpAccount.isNull());
    QTRY_VERIFY(mTpAccount->isReady(Tp::Account::FeatureCore));

    mAccount = qobject_cast<OfonoAccountEntry*>(AccountEntryFactory::createEntry(mTpAccount, this));
    QVERIFY(mAccount);

    // make sure the connection is available
    QTRY_VERIFY(!mTpAccount->connection().isNull());
    QTRY_COMPARE(mTpAccount->connection()->selfContact()->presence().type(), Tp::ConnectionPresenceTypeAvailable);

    // and create the mock controller
    mMockController = new MockController("ofono", this);

    // just in case, wait some time
    QTest::qWait(1000);
}

void CallEntryTest::cleanup()
{
    doCleanup();

    mMockController->deleteLater();
    mAccount->deleteLater();
}

void CallEntryTest::testIsVoicemail_data()
{
    QTest::addColumn<QString>("voicemailNumber");
    QTest::addColumn<QString>("callNumber");
    QTest::addColumn<bool>("isVoicemail");

    QTest::newRow("calling voicemail") << "*123" << "*123" << true;
    QTest::newRow("calling other number") << "*1234" << "5555555" << false;
}

void CallEntryTest::testIsVoicemail()
{
    QFETCH(QString, voicemailNumber);
    QFETCH(QString, callNumber);
    QFETCH(bool, isVoicemail);

    mMockController->setVoicemailNumber(voicemailNumber);
    QTRY_COMPARE(mAccount->voicemailNumber(), voicemailNumber);

    // now place a call to a number that is not the voicemail number
    QVariantMap properties;
    properties["Caller"] = callNumber;
    properties["State"] = "incoming";
    mMockController->placeCall(properties);
    QTRY_VERIFY(!mCallChannel.isNull());
    QCOMPARE(mCallChannel->targetContact()->id(), callNumber);

    CallEntry *callEntry = new CallEntry(mCallChannel);
    QCOMPARE(callEntry->isVoicemail(), isVoicemail);

    callEntry->endCall();
    callEntry->deleteLater();
    mCallChannel = Tp::CallChannelPtr();
}

void CallEntryTest::onCallChannelAvailable(const Tp::CallChannelPtr &channel)
{
    mCallChannel = channel;
}

QTEST_MAIN(CallEntryTest)
#include "CallEntryTest.moc"
