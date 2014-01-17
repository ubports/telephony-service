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

#ifndef VOICEMAILINDICATOR_H
#define VOICEMAILINDICATOR_H

#include <QDBusInterface>
#include <QVariantMap>
#include <QStringList>

class VoiceMailIndicator : public QObject
{
    Q_OBJECT
public:
    explicit VoiceMailIndicator(QObject *parent = 0);

    void showVoicemailOnApp();

public Q_SLOTS:
    void onVoicemailCountChanged(uint count);
    void onVoicemailIndicatorChanged(bool active);
    void onAccountReady();

private:
    bool voicemailIndicatorVisible();
    uint voicemailCount();
    bool checkConnected();
    QDBusConnection mConnection;
};

#endif // VOICEMAILINDICATOR_H
