/**
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 */

#ifndef DBUSTYPES
#define DBUSTYPES

#include <QDBusObjectPath>
#include <QDBusArgument>

struct MessageStruct {
    QDBusObjectPath path;
    QVariantMap properties;
};

struct AttachmentStruct {
    QString id;
    QString contentType;
    QString filePath;
};

struct ProtocolStruct {
    QString name;
    uint features;
    QString fallbackProtocol;
    uint fallbackMatchRule;
    QString fallbackSourceProperty;
    QString fallbackDestinationProperty;
    bool showOnSelector;
    bool showOnlineStatus;
    QString backgroundImage;
    QString icon;
    QString serviceName;
    QString serviceDisplayName;
    bool joinExistingChannels;
    bool returnToSend;
    bool enableAttachments;
    bool enableRejoin;
    bool enableTabCompletion;
    bool leaveRoomsOnClose;
    bool enableChatStates;
};

typedef QList<AttachmentStruct> AttachmentList;
Q_DECLARE_METATYPE(AttachmentStruct)
Q_DECLARE_METATYPE(AttachmentList)

typedef QList<MessageStruct> MessageList;
Q_DECLARE_METATYPE(MessageStruct)
Q_DECLARE_METATYPE(MessageList)

typedef QList<ProtocolStruct> ProtocolList;
Q_DECLARE_METATYPE(ProtocolStruct)
Q_DECLARE_METATYPE(ProtocolList)

QDBusArgument &operator<<(QDBusArgument &argument, const AttachmentStruct &attachment);
const QDBusArgument &operator>>(const QDBusArgument &argument, AttachmentStruct &attachment);

QDBusArgument &operator<<(QDBusArgument &argument, const ProtocolStruct &protocol);
const QDBusArgument &operator>>(const QDBusArgument &argument, ProtocolStruct &protocol);

#endif
