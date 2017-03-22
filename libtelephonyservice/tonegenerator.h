/*
 * Copyright (C) 2014-2017 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Martti Piirainen <martti.piirainen@canonical.com>
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

#ifndef TONEGENERATOR_H
#define TONEGENERATOR_H

#include <QObject>

class QTimer;

static const int DTMF_LOCAL_PLAYBACK_DURATION = 200; /* in milliseconds */
static const int WAITING_PLAYBACK_DURATION = 8000; /* in milliseconds */
static const uint WAITING_TONE = 79;
static const uint CALL_ENDED_TONE = 257;
static const uint DIALING_TONE = 66;
static const uint RINGING_TONE = 70;

class ToneGenerator : public QObject
{
    Q_OBJECT
public:
    ~ToneGenerator();
    static ToneGenerator *instance();

public Q_SLOTS:
    /**
     * Valid tones: 0..9 (number keys), 10 (*), 11 (#)
     */
    void playDTMFTone(uint key);
    void playWaitingTone();
    void stopWaitingTone();
    void playCallEndedTone();
    void playDialingTone();
    void playRingingTone();
    void stopTone();

private Q_SLOTS:
    void stopDTMFTone();
    bool startEventTone(uint key);

private:
    explicit ToneGenerator(QObject *parent = 0);
    QTimer* mDTMFPlaybackTimer;
    QTimer* mWaitingPlaybackTimer;
};

#endif // TONEGENERATOR_H
