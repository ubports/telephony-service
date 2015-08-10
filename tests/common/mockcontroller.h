/**
 * Copyright (C) 2013 Canonical, Ltd.
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
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 */

#ifndef MOCKCONTROLLER_H
#define MOCKCONTROLLER_H

#include <QObject>
#include "MockConnectionInterface.h"

class MockController : public ComCanonicalMockConnectionInterface
{
    Q_OBJECT
public:
    explicit MockController(const QString &protocol, QObject *parent = 0);

public Q_SLOTS:
    // We only reimplement the methods that need sync replies
    QString placeCall(const QVariantMap &properties);
    QString serial();

private:
    QString mProtocol;
    QString mMockObject;
};

#endif // MOCKCONTROLLER_H
