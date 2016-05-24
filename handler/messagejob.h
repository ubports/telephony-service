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

#ifndef MESSAGEJOB_H
#define MESSAGEJOB_H

#include <QObject>

class MessageJob : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool isFinished READ isFinished NOTIFY isFinishedChanged)
    Q_ENUMS(Status)
public:
    enum Status {
        Pending,
        Initialising,
        Running,
        Finished,
        Failed
    };

    explicit MessageJob(QObject *parent = 0);
    virtual ~MessageJob();

    Status status() const;
    bool isFinished() const;

    void waitForFinished(int timeout = 10000);

Q_SIGNALS:
    void statusChanged();
    void isFinishedChanged();
    void finished();

public Q_SLOTS:
    virtual void startJob();

protected:
    void setStatus(Status status);
    void scheduleDeletion(int timeout = 60000);

private:
    Status mStatus;
    bool mFinished;
};

#endif // MESSAGEJOB_H
