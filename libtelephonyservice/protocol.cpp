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

#include "protocol.h"
#include "dbustypes.h"
#include <QFileInfo>
#include <QSettings>

Protocol::Protocol(const ProtocolStruct & protocol)
    : mName(protocol.name)
    , mFeatures(protocol.features)
    , mFallbackProtocol(protocol.fallbackProtocol)
    , mBackgroundImage(protocol.backgroundImage)
    , mIcon(protocol.icon)
    , mServiceName(protocol.serviceName)
{
}

Protocol::Protocol(const QString &name, Features features, const QString &fallbackProtocol, const QString &backgroundImage, const QString &icon, const QString &serviceName, QObject *parent)
: QObject(parent), mName(name), mFeatures(features), mFallbackProtocol(fallbackProtocol), mBackgroundImage(backgroundImage), mIcon(icon), mServiceName(serviceName)
{
}

ProtocolStruct Protocol::dbusType()
{
    return ProtocolStruct{mName, static_cast<uint>(mFeatures), mFallbackProtocol, mBackgroundImage, mIcon, mServiceName};
}

QString Protocol::name() const
{
    return mName;
}

QString Protocol::icon() const
{
    return mIcon;
}

QString Protocol::serviceName() const
{
    return mServiceName;
}

Protocol::Features Protocol::features() const
{
    return mFeatures;
}

QString Protocol::fallbackProtocol() const
{
    return mFallbackProtocol;
}

QString Protocol::backgroundImage() const
{
    return mBackgroundImage;
}

Protocol *Protocol::fromFile(const QString &fileName)
{
    QFileInfo file(fileName);
    if (!file.isReadable() || file.suffix() != "protocol") {
        return 0;
    }

    QString protocolName = file.baseName();
    QSettings settings(fileName, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    settings.beginGroup("Protocol");
    QString name = settings.value("Name", protocolName).toString();
    QStringList featureList = settings.value("Features").toStringList();
    Protocol::Features features;
    Q_FOREACH(const QString &feature, featureList) {
        if (feature == "text") {
            features |= Protocol::TextChats;
        } else if (feature == "voice") {
            features |= Protocol::VoiceCalls;
        }
    }
    QString fallbackProtocol = settings.value("FallbackProtocol").toString();
    QString backgroundImage = settings.value("BackgroundImage").toString();
    QString icon = settings.value("Icon").toString();
    QString serviceName = settings.value("ServiceName").toString();

    return new Protocol(name, features, fallbackProtocol, backgroundImage, icon, serviceName);
}
