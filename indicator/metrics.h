/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#ifndef METRICS_H
#define METRICS_H

#include <QObject>
#include <QMap>
#include <libusermetricsinput/MetricManager.h>

using namespace UserMetricsInput;

class Metrics : public QObject
{
    Q_OBJECT
public:
    enum MetricType {
        SentMessages,
        ReceivedMessages,
        IncomingCalls,
        OutgoingCalls,
        CallDurations
    };

    static Metrics *instance();
    void increment(MetricType metric, double amount = 1.0f);
    
private:
    explicit Metrics(QObject *parent = 0);
    QMap<MetricType, UserMetricsInput::MetricPtr> mMetrics;
    UserMetricsInput::MetricManagerPtr mMetricManager;
};

#endif // METRICS_H
