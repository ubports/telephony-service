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

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "dbustypes.h"
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

    /// @brief the strategy to be used when matching fallback accounts
    Q_PROPERTY(MatchRule fallbackMatchRule READ fallbackMatchRule CONSTANT)

    /// @brief the property to be used on this protocol to match the fallback account
    Q_PROPERTY(QString fallbackSourceProperty READ fallbackSourceProperty CONSTANT)

    /// @brief the property to be used on the fallback protocol to match the account
    Q_PROPERTY(QString fallbackDestinationProperty READ fallbackDestinationProperty CONSTANT)

    /// @brief whether accounts from this protocol should be shown on account selectors
    Q_PROPERTY(bool showOnSelector READ showOnSelector CONSTANT)

    /// @brief whether the online status of contacts for this account should be shown on UI
    Q_PROPERTY(bool showOnlineStatus READ showOnlineStatus CONSTANT)

    /// @brief the file path for the image that represents this protocol
    Q_PROPERTY(QString backgroundImage READ backgroundImage CONSTANT)

    /// @brief the file path for the image that represents this protocol
    Q_PROPERTY(QString icon READ icon CONSTANT)

    /// @brief the title that represents this protocol
    Q_PROPERTY(QString serviceName READ serviceName CONSTANT)

    /// @brief the name to display for this protocol
    Q_PROPERTY(QString serviceDisplayName READ serviceDisplayName CONSTANT)

    /// @brief whether this protocol allows joining existing channels
    Q_PROPERTY(bool joinExistingChannels READ joinExistingChannels CONSTANT)

    /// @brief whether this protocol should send the message directly when return is pressed
    Q_PROPERTY(bool returnToSend READ returnToSend CONSTANT)

    /// @brief whether this protocol supports attachments
    Q_PROPERTY(bool enableAttachments READ enableAttachments CONSTANT)

    /// @brief whether this protocol supports rejoin channels manually
    Q_PROPERTY(bool enableRejoin READ enableRejoin CONSTANT)

    /// @brief whether this protocol supports tab completion
    Q_PROPERTY(bool enableTabCompletion READ enableTabCompletion CONSTANT)

    /// @brief whether the app needs to request leave all room channels when the app closes
    Q_PROPERTY(bool leaveRoomsOnClose READ leaveRoomsOnClose CONSTANT)

    /// @brief whether the app support chat states as typing notifications
    Q_PROPERTY(bool enableChatStates READ enableChatStates CONSTANT)

public:
    enum Feature {
        TextChats = 0x1,
        VoiceCalls = 0x2,
        AllFeatures = (TextChats | VoiceCalls)
    };
    Q_DECLARE_FLAGS(Features, Feature)

    Protocol(const ProtocolStruct& protocolStruct);

    enum MatchRule {
        MatchAny,
        MatchProperties
    };

    QString name() const;
    Features features() const;
    QString fallbackProtocol() const;
    MatchRule fallbackMatchRule() const;
    QString fallbackSourceProperty() const;
    QString fallbackDestinationProperty() const;
    bool showOnSelector() const;
    bool showOnlineStatus() const;
    QString backgroundImage() const;
    QString icon() const;
    QString serviceName() const;
    QString serviceDisplayName() const;
    bool joinExistingChannels() const;
    bool returnToSend() const;
    bool enableAttachments() const;
    bool enableRejoin() const;
    bool enableTabCompletion() const;
    bool leaveRoomsOnClose() const;
    bool enableChatStates() const;

    static Protocol *fromFile(const QString &fileName);
    ProtocolStruct dbusType();

    friend class ProtocolManager;

protected:
    explicit Protocol(const QString &name, Features features,
                      const QString &fallbackProtocol = QString::null,
                      const MatchRule fallbackMatchRule = MatchAny,
                      const QString &fallbackSourceProperty = QString::null,
                      const QString &fallbackDestinationProperty = QString::null,
                      bool showOnSelector = true,
                      bool showOnlineStatus = false,
                      const QString &backgroundImage = QString::null,
                      const QString &icon = QString::null,
                      const QString &serviceName = QString::null,
                      const QString &serviceDisplayName = QString::null,
                      bool joinExistingChannels = false,
                      bool returnToSend = false,
                      bool enableAttachments = true,
                      bool enableRejoin = false,
                      bool enableTabCompletion = false,
                      bool leaveRoomsOnClose = false,
                      bool enableChatStates = false,
                      QObject *parent = 0);

private:
    QString mName;
    Features mFeatures;
    QString mFallbackProtocol;
    MatchRule mFallbackMatchRule;
    QString mFallbackSourceProperty;
    QString mFallbackDestinationProperty;
    bool mShowOnSelector;
    bool mShowOnlineStatus;
    QString mBackgroundImage;
    QString mIcon;
    QString mServiceName;
    QString mServiceDisplayName;
    bool mJoinExistingChannels;
    bool mReturnToSend;
    bool mEnableAttachments;
    bool mEnableRejoin;
    bool mEnableTabCompletion;
    bool mLeaveRoomsOnClose;
    bool mEnableChatStates;
};

class Protocols : public QList<Protocol*>
{
public:
    ProtocolList dbusType() {
        // return list of DBus types
        ProtocolList protocolList;
        Q_FOREACH(Protocol *protocol, *this) {
            protocolList << protocol->dbusType();
        }
        return protocolList;
    }
};

#endif // PROTOCOL_H
