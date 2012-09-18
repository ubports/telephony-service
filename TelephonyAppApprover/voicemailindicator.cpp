/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "voicemailindicator.h"
#include "telepathyhelper.h"
#include <QDebug>
#include <QDBusReply>
#include <qindicateserver.h>
#include <qindicateindicator.h>

#define CANONICAL_IFACE_TELEPHONY "com.canonical.Telephony"

VoiceMailIndicator::VoiceMailIndicator(QObject *parent)
: QObject(parent),
  mConnection(QDBusConnection::sessionBus())
{
    mIndicateServer = new QIndicate::Server(this, "/com/canonical/TelephonyApp/indicators/voicemail");
    mIndicateServer->setType("message");
    mIndicateServer->setDesktopFile("/usr/share/applications/telephony-app-phone.desktop");
    mIndicateServer->show();

    mIndicator = new QIndicate::Indicator(this);
    mIndicator->setNameProperty("Voicemail");

    // the indicator gets automatically added to the default server, so we need to remove it from there
    // and add to the correct server
    QIndicate::Server::defaultInstance()->removeIndicator(mIndicator);
    mIndicateServer->addIndicator(mIndicator);

    connect(mIndicator,
            SIGNAL(display(QIndicate::Indicator*)),
            SLOT(onIndicatorDisplay(QIndicate::Indicator*)));

    if(!checkConnected()) {
        connect(TelepathyHelper::instance(), SIGNAL(accountReady()), SLOT(onAccountReady()));
        connect(TelepathyHelper::instance(), SIGNAL(connectionChanged()), SLOT(onAccountReady()));
    } else {
        onAccountReady();
    }
}

bool VoiceMailIndicator::checkConnected()
{
    return TelepathyHelper::instance()->account() && TelepathyHelper::instance()->account()->connection();
}

void VoiceMailIndicator::onAccountReady()
{
    if (!checkConnected()) {
        return;
    }

    Tp::ConnectionPtr conn(TelepathyHelper::instance()->account()->connection());
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    mConnection.connect(busName, objectPath, CANONICAL_IFACE_TELEPHONY, QLatin1String("VoicemailCountChanged"),
                        this, SLOT(onVoicemailCountChanged(int)));

    mConnection.connect(busName, objectPath, CANONICAL_IFACE_TELEPHONY, QLatin1String("VoicemailIndicatorChanged"),
                        this, SLOT(onVoicemailIndicatorChanged(bool)));

    onVoicemailCountChanged(voicemailCount());
}

void VoiceMailIndicator::onVoicemailIndicatorChanged(bool active)
{
    if (active) {
        mIndicator->setCountProperty(voicemailCount());
        mIndicator->show();
        mIndicator->setDrawAttentionProperty(true);
    } else {
        mIndicator->hide();
        mIndicator->setDrawAttentionProperty(false);
    }
}

bool VoiceMailIndicator::voicemailIndicatorVisible()
{
    if (!checkConnected()) {
        return false;
    }

    Tp::ConnectionPtr conn(TelepathyHelper::instance()->account()->connection());
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    QDBusInterface connIface(busName, objectPath, CANONICAL_IFACE_TELEPHONY);
    QDBusReply<bool> reply = connIface.call("VoicemailIndicator");
    if (reply.isValid()) {
        return reply.value();
    }
    return false;
}

int VoiceMailIndicator::voicemailCount()
{
    if (!checkConnected()) {
        return 0;
    }

    Tp::ConnectionPtr conn(TelepathyHelper::instance()->account()->connection());
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    QDBusInterface connIface(busName, objectPath, CANONICAL_IFACE_TELEPHONY);
    QDBusReply<int> reply = connIface.call("VoicemailCount");
    if (reply.isValid()) {
        return reply.value();
    }
    return false;
}

void VoiceMailIndicator::onVoicemailCountChanged(int count)
{
    if (voicemailIndicatorVisible()) {
        mIndicator->setCountProperty(count);
        mIndicator->show();
        mIndicator->setDrawAttentionProperty(true);
    } else {
        mIndicator->hide();
        mIndicator->setDrawAttentionProperty(false);
    }
}

void VoiceMailIndicator::onIndicatorDisplay(QIndicate::Indicator *indicator)
{
    QDBusInterface telephonyApp("com.canonical.TelephonyApp",
                                "/com/canonical/TelephonyApp",
                                "com.canonical.TelephonyApp");
    telephonyApp.call("ShowVoicemail");
}
