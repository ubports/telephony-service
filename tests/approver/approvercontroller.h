/**
 * Copyright (C) 2013-2015 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 */

#ifndef APPROVERCONTROLLER_H
#define APPROVERCONTROLLER_H

#include <QObject>
#include <QDBusInterface>

class ApproverController : public QObject
{
    Q_OBJECT
public:
    static ApproverController *instance();

public Q_SLOTS:
    void acceptCall();
    void hangUpAndAcceptCall();

private:
    explicit ApproverController(QObject *parent = 0);
    QDBusInterface mApproverInterface;
};

#endif // ApproverController_H
