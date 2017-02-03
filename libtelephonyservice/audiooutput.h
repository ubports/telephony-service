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

#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QObject>
#include <QDBusArgument>

struct AudioOutputDBus {
    QString id;
    QString type;
    QString name;
};
Q_DECLARE_METATYPE(AudioOutputDBus)

typedef QList<AudioOutputDBus> AudioOutputDBusList;
Q_DECLARE_METATYPE(AudioOutputDBusList)

class AudioOutput : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString type READ type CONSTANT)

public:
    AudioOutput(const QString &id, const QString &name, const QString &type, QObject *parent = 0);
    QString id() const;
    QString name() const;
    QString type() const;

private:
    QString mId;
    QString mName;
    QString mType;
};

QDBusArgument &operator<<(QDBusArgument &argument, const AudioOutputDBus &output);
const QDBusArgument &operator>>(const QDBusArgument &argument, AudioOutputDBus &output);

#endif // AUDIOOUTPUT_H
