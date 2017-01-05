/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
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
    , mFallbackMatchRule((MatchRule)protocol.fallbackMatchRule)
    , mFallbackSourceProperty(protocol.fallbackSourceProperty)
    , mFallbackDestinationProperty(protocol.fallbackDestinationProperty)
    , mShowOnSelector(protocol.showOnSelector)
    , mShowOnlineStatus(protocol.showOnlineStatus)
    , mBackgroundImage(protocol.backgroundImage)
    , mIcon(protocol.icon)
    , mServiceName(protocol.serviceName)
    , mServiceDisplayName(protocol.serviceDisplayName)
    , mJoinExistingChannels(protocol.joinExistingChannels)
    , mReturnToSend(protocol.returnToSend)
{
}

Protocol::Protocol(const QString &name,
                   Features features,
                   const QString &fallbackProtocol,
                   MatchRule fallbackMatchRule,
                   const QString &fallbackSourceProperty,
                   const QString &fallbackDestinationProperty,
                   bool showOnSelector,
                   bool showOnlineStatus,
                   const QString &backgroundImage,
                   const QString &icon,
                   const QString &serviceName,
                   const QString &serviceDisplayName,
                   bool joinExistingChannels,
                   bool returnToSend,
                   QObject *parent)
 : QObject(parent)
 , mName(name)
 , mFeatures(features)
 , mFallbackProtocol(fallbackProtocol)
 , mFallbackMatchRule(fallbackMatchRule)
 , mFallbackSourceProperty(fallbackSourceProperty)
 , mFallbackDestinationProperty(fallbackDestinationProperty)
 , mShowOnSelector(showOnSelector)
 , mShowOnlineStatus(showOnlineStatus)
 , mBackgroundImage(backgroundImage)
 , mIcon(icon)
 , mServiceName(serviceName)
 , mServiceDisplayName(serviceDisplayName)
 , mJoinExistingChannels(joinExistingChannels)
 , mReturnToSend(returnToSend)
{
}

ProtocolStruct Protocol::dbusType()
{
    return ProtocolStruct{mName, static_cast<uint>(mFeatures), mFallbackProtocol, static_cast<uint>(mFallbackMatchRule), mFallbackSourceProperty, mFallbackDestinationProperty, mShowOnSelector, mShowOnlineStatus, mBackgroundImage, mIcon, mServiceName, mServiceDisplayName, mJoinExistingChannels, mReturnToSend};
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

Protocol::MatchRule Protocol::fallbackMatchRule() const
{
    return mFallbackMatchRule;
}

QString Protocol::fallbackSourceProperty() const
{
    return mFallbackSourceProperty;
}

QString Protocol::fallbackDestinationProperty() const
{
    return mFallbackDestinationProperty;
}

bool Protocol::showOnSelector() const
{
    return mShowOnSelector;
}

bool Protocol::showOnlineStatus() const
{
    return mShowOnlineStatus;
}

QString Protocol::backgroundImage() const
{
    return mBackgroundImage;
}

QString Protocol::serviceDisplayName() const
{
    return mServiceDisplayName;
}

bool Protocol::joinExistingChannels() const
{
    return mJoinExistingChannels;
}

bool Protocol::returnToSend() const
{
    return mReturnToSend;
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
    QString matchRuleString = settings.value("FallbackMatchRule").toString();
    MatchRule matchRule = MatchAny;
    if (matchRuleString == "match_any") {
        matchRule = MatchAny;
    } else if (matchRuleString == "match_properties") {
        matchRule = MatchProperties;
    }
    QString fallbackSourceProperty = settings.value("FallbackSourceProperty").toString();
    QString fallbackDestinationProperty = settings.value("FallbackDestinationProperty").toString();
    bool showOnSelector = settings.value("ShowOnSelector", true).toBool();
    bool showOnlineStatus = settings.value("ShowOnlineStatus", false).toBool();
    QString backgroundImage = settings.value("BackgroundImage").toString();
    QString icon = settings.value("Icon").toString();
    QString serviceName = settings.value("ServiceName").toString();
    QString serviceDisplayName = settings.value("ServiceDisplayName").toString();
    bool joinExistingChannels = settings.value("JoinExistingChannels").toBool();
    bool returnToSend = settings.value("ReturnToSend").toBool();

    return new Protocol(name, features, fallbackProtocol, matchRule, fallbackSourceProperty, fallbackDestinationProperty,
                        showOnSelector, showOnlineStatus, backgroundImage, icon, serviceName, serviceDisplayName, joinExistingChannels, returnToSend);
}
