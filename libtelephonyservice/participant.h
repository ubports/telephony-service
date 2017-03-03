/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include "contactwatcher.h"

class Participant : public ContactWatcher
{
    Q_OBJECT
    Q_PROPERTY(uint roles READ roles NOTIFY rolesChanged)
public:
    explicit Participant(const QString &identifier, uint roles, uint handle, const QString &avatar = QString(), uint state = 0, QObject *parent = 0);
    explicit Participant(QObject *parent = 0);
    explicit Participant(const Participant &other);
    ~Participant();

    void setRoles(uint roles);
    uint roles() const;
    uint handle() const;
    QString avatar() const;
    uint state() const;

Q_SIGNAL
    void rolesChanged();

private:
    uint mRoles;
    uint mHandle;
    QString mAvatar;
    uint mState;
};

#endif // PARTICIPANT_H
