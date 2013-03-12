/*
 * Copyright 2012-2013 Canonical Ltd.
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Ubuntu.Components 0.1
import "../dateUtils.js" as DateUtils

Item {
    id: communicationDelegate
    property string itemType
    property string message: "Test"
    property alias itemIcon: icon.source
    property bool incoming: false
    property variant timestamp
    property bool missed: false
    property variant item

    signal clicked(var mouse)

    anchors.left: parent ? parent.left : undefined
    anchors.right: parent ? parent.right: undefined

    height: bubble.height + units.gu(1)

    Component.onCompleted: {
        if (item.newItem) {
            item.newItem = false;
        }
    }

    BorderImage {
        id: bubble

        anchors.left: incoming ? icon.right : parent.left
        anchors.leftMargin: units.gu(1)
        anchors.right: incoming ? parent.right : icon.left
        anchors.rightMargin: units.gu(1)
        anchors.top: parent.top

        function selectBubble() {
            var fileName = "../assets/conversation_";
            if (itemType == "call") {
                fileName += "phonecall_";
            }
            if (incoming) {
                fileName += "incoming.sci";
            } else {
                fileName += "outgoing.sci";
            }
            return fileName;
        }

        source: selectBubble()

        height: messageText.height + units.gu(3)

        Label {
            id: messageText

            anchors.verticalCenter: parent.verticalCenter
            anchors.left: bubble.left
            anchors.leftMargin: bubble.border.left
            anchors.right: bubble.right
            anchors.rightMargin: bubble.border.right
            height: paintedHeight

            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            fontSize: "medium"
            color: (incoming && itemType != "call") ? "#ffffff" : "#333333"
            opacity: (incoming && itemType != "call") ? 1 : 0.9
            text: selectMessage()

            function selectMessage() {
                if (communicationDelegate.itemType == "call") {
                    if (missed) {
                        return "missed call";
                    } else {
                        return DateUtils.formatFriendlyCallDuration(item.duration) + " call";
                    }
                } else {
                    return communicationDelegate.message;
                }
            }
        }
    }

    Image {
        id: icon
        width: units.gu(2)
        height: units.gu(2)

        anchors.left: incoming ? parent.left : undefined
        anchors.leftMargin: units.gu(2)
        anchors.right: incoming ? undefined : parent.right
        anchors.rightMargin: units.gu(2)
        anchors.top: parent.top
        anchors.topMargin: units.gu(1)
    }

    MouseArea {
        anchors.fill: parent
        onClicked: communicationDelegate.clicked(mouse)
    }
}
