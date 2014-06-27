/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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

#include "voicemailindicator.h"
#include "telepathyhelper.h"
#include "messagingmenu.h"
#include <QDebug>
#include <QDBusReply>

VoiceMailIndicator::VoiceMailIndicator(QObject *parent)
: QObject(parent),
  mConnection(QDBusConnection::sessionBus())
{
    connect(TelepathyHelper::instance(), SIGNAL(accountReady()), SLOT(onAccountReady()));
    connect(TelepathyHelper::instance(), SIGNAL(connectedChanged()), SLOT(onAccountReady()));
}

bool VoiceMailIndicator::checkConnected()
{
    return TelepathyHelper::instance()->connected();
}

void VoiceMailIndicator::onAccountReady()
{
    if (!checkConnected()) {
        return;
    }

    // FIXME: handle multiple accounts
    Tp::ConnectionPtr conn(TelepathyHelper::instance()->accounts()[0]->connection());
    if (conn.isNull()) {
        return;
    }

    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    mConnection.connect(busName, objectPath, CANONICAL_TELEPHONY_VOICEMAIL_IFACE, QLatin1String("VoicemailCountChanged"),
                        this, SLOT(onVoicemailCountChanged(uint)));

    mConnection.connect(busName, objectPath, CANONICAL_TELEPHONY_VOICEMAIL_IFACE, QLatin1String("VoicemailIndicatorChanged"),
                        this, SLOT(onVoicemailIndicatorChanged(bool)));

    onVoicemailCountChanged(voicemailCount());
}

void VoiceMailIndicator::onVoicemailIndicatorChanged(bool active)
{
    if (active) {
        MessagingMenu::instance()->showVoicemailEntry(voicemailCount());
    } else {
        MessagingMenu::instance()->hideVoicemailEntry();
    }
}

bool VoiceMailIndicator::voicemailIndicatorVisible()
{
    if (!checkConnected()) {
        return false;
    }

    // FIXME: handle multiple accounts
    Tp::ConnectionPtr conn(TelepathyHelper::instance()->accounts()[0]->connection());
    if (conn.isNull()) {
        return false;
    }

    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    QDBusInterface connIface(busName, objectPath, CANONICAL_TELEPHONY_VOICEMAIL_IFACE);
    QDBusReply<bool> reply = connIface.call("VoicemailIndicator");
    if (reply.isValid()) {
        return reply.value();
    }
    return false;
}

uint VoiceMailIndicator::voicemailCount()
{
    if (!checkConnected()) {
        return 0;
    }

    // FIXME: handle multiple accounts
    Tp::ConnectionPtr conn(TelepathyHelper::instance()->accounts()[0]->connection());
    if (conn.isNull()) {
        return false;
    }

    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    QDBusInterface connIface(busName, objectPath, CANONICAL_TELEPHONY_VOICEMAIL_IFACE);
    QDBusReply<uint> reply = connIface.call("VoicemailCount");
    if (reply.isValid()) {
        return reply.value();
    }
    return 0;
}

void VoiceMailIndicator::onVoicemailCountChanged(uint count)
{
    onVoicemailIndicatorChanged(voicemailIndicatorVisible());
}
