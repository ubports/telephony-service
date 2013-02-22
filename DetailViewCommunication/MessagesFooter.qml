/*
 * Copyright 2012 Canonical Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Ubuntu.Components 0.1
import TelephonyApp 0.1

FocusScope {
    id: footer
   
    property bool validRecipient: false
    property alias hasFocus: entry.activeFocus
    property bool newConversation: true
    onFocusChanged: {
        if (focus && entryFocusScope && entryFocusScope.inputEntry) {
            entryFocusScope.inputEntry.forceActiveFocus()
        }
    }

    signal newMessage(string message)

    height: visible ? entry.height + units.gu(2) : 0

    Rectangle {
        anchors.fill: parent
        color: "white"
        opacity: 0.3
    }

    Rectangle {
        id: separator

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: units.dp(1)
        color: "white"
    }

    FocusScope {
        id: entryFocusScope
        property var inputEntry: entry
        anchors.top: separator.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        MouseArea {
            anchors.fill: entry
            onClicked: entry.forceActiveFocus()
            z: 1
        }

        TextArea {
            id: entry

            objectName: "newMessageText"
            anchors.left: parent.left
            anchors.leftMargin: units.gu(1)
            anchors.right: sendButton.left
            anchors.rightMargin: units.gu(1)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: units.gu(1.5)
            focus: true
            font.pixelSize: FontUtils.sizeToPixels("medium")
            font.family: "Ubuntu"

            // send message if return was pressed
            Keys.onReturnPressed: sendButton.clicked(null)
            Keys.onEscapePressed: text = ""
            height: units.gu(4)
            placeholderText: newConversation ? "Compose" : "Reply via SMS"
            autoExpand: true
            maximumLineCount: 0
        }

        Button {
            id: sendButton

            anchors.right: parent.right
            anchors.rightMargin: units.gu(1)
            anchors.bottom: entry.bottom
            width: units.gu(9)
            height: units.gu(4)
            enabled: validRecipient && entry.text != "" && telepathyHelper.connected
            color: "#37b301"
            text: "Send"
            objectName: "sendMessageButton"
            onClicked: {
                if(!validRecipient) {
                    return;
                }
                footer.newMessage(entry.text)
                entry.text = ""
            }
        }
    }
}
