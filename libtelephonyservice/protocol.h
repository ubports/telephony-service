/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QObject>

/// @brief describes one protocol and the features it supports
class Protocol : public QObject
{
    Q_OBJECT
    /// @brief the name of the protocol
    Q_PROPERTY(QString name READ name CONSTANT)

    /// @brief the features this protocol supports
    Q_PROPERTY(Features features READ features CONSTANT)

    /// @brief the fallback protocol to be used for operations that support it (mainly text features)
    Q_PROPERTY(QString fallbackProtocol READ fallbackProtocol CONSTANT)

    /// @brief the file path for the image that represents this protocol
    Q_PROPERTY(QString backgroundImage READ backgroundImage CONSTANT)

    /// @brief the file path for the image that represents this protocol
    Q_PROPERTY(QString icon READ icon CONSTANT)

    /// @brief the title that represents this protocol
    Q_PROPERTY(QString serviceName READ serviceName CONSTANT)

    /// @brief the name to display for this protocol
    Q_PROPERTY(QString serviceDisplayName READ serviceDisplayName CONSTANT)

public:
    enum Feature {
        TextChats = 0x1,
        VoiceCalls = 0x2
    };
    Q_DECLARE_FLAGS(Features, Feature)

    QString name() const;
    Features features() const;
    QString fallbackProtocol() const;
    QString backgroundImage() const;
    QString icon() const;
    QString serviceName() const;
    QString serviceDisplayName() const;

    static Protocol *fromFile(const QString &fileName);

    friend class ProtocolManager;

protected:
    explicit Protocol(const QString &name, Features features, const QString &fallbackProtocol = QString::null, const QString &backgroundImage = QString::null, const QString &icon = QString::null, const QString &serviceName = QString::null, const QString &serviceDisplayName = QString::null, QObject *parent = 0);

private:
    QString mName;
    Features mFeatures;
    QString mFallbackProtocol;
    QString mBackgroundImage;
    QString mIcon;
    QString mServiceName;
    QString mServiceDisplayName;
};

typedef QList<Protocol*> Protocols;

#endif // PROTOCOL_H
