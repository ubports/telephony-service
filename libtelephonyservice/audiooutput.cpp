/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
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

#include "audiooutput.h"

QDBusArgument &operator<<(QDBusArgument &argument, const AudioOutputDBus &output)
{
    argument.beginStructure();
    argument << output.id << output.type << output.name;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, AudioOutputDBus &output)
{
    argument.beginStructure();
    argument >> output.id >> output.type >> output.name;
    argument.endStructure();
    return argument;
}


AudioOutput::AudioOutput(const QString& id, const QString& name, const QString& type, QObject *parent) :
    QObject(parent), mId(id), mName(name), mType(type)
{
}

QString AudioOutput::id() const
{
    return mId;
}

QString AudioOutput::name() const
{
    return mName;
}

QString AudioOutput::type() const
{
    return mType;
}
