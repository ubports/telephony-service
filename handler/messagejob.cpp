/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "messagejob.h"
#include "handlerdbus.h"
#include <QCoreApplication>
#include <QTime>
#include <QTimer>
#include <QDebug>

MessageJob::MessageJob(QObject *parent)
: QObject(parent), mStatus(Pending), mFinished(false), mAdaptor(0)
{
}

MessageJob::~MessageJob()
{
    HandlerDBus::instance()->unregisterObject(mObjectPath);
}

void MessageJob::setAdaptorAndRegister(QDBusAbstractAdaptor *adaptor)
{
    mAdaptor = adaptor;

    static ulong count = 0;
    // just to avoid overflowing
    if (count == ULONG_MAX) {
        count = 0;
    }
    mObjectPath = HandlerDBus::instance()->registerObject(this, QString("messagejob%1").arg(count++));
}

MessageJob::Status MessageJob::status() const
{
    return mStatus;
}

bool MessageJob::isFinished() const
{
    return mFinished;
}

QString MessageJob::objectPath() const
{
    return mObjectPath;
}

void MessageJob::waitForFinished(int timeout)
{
    QTime time;
    time.start();
    while (!mFinished && time.elapsed() < timeout) {
        QCoreApplication::processEvents();
    }
}

void MessageJob::startJob()
{
    // the default implementation just sets the status to Finished
    setStatus(Finished);
}

void MessageJob::setStatus(MessageJob::Status status)
{
    mStatus = status;
    Q_EMIT statusChanged();

    // update the isFinished property too
    bool wasFinished = mFinished;
    mFinished = mStatus == Finished || mStatus == Failed;
    if (wasFinished != mFinished) {
        Q_EMIT isFinishedChanged();
    }
    if (mFinished) {
        Q_EMIT finished();
    }
}

void MessageJob::scheduleDeletion(int timeout)
{
    QTimer::singleShot(timeout, this, &QObject::deleteLater);
}

