/*
 * Copyright 2012 Canonical Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import TelephonyApp 0.1
import "../Widgets" as LocalWidgets
import Ubuntu.Components.ListItems 0.1 as ListItem
import Ubuntu.Components 0.1

LocalWidgets.TelephonyPage {
    id: messageList
    objectName: "communicationPanel"
    title: "Conversations"
    chromeButtons: ListModel {
        ListElement {
            label: "Compose"
            name: "compose"
            icon: "../assets/compose.png"
        }
    }

    onChromeButtonClicked: {
        if (buttonName == "compose") {
            telephony.startNewMessage()
        }
    }

    anchors.fill: parent

    LocalWidgets.Header {
        id: header
        text: title
    }

    TextField {
        id: search
        objectName: "messageSearchBox"

        anchors.top: header.bottom
        anchors.topMargin: units.gu(1)
        anchors.left: parent.left
        anchors.leftMargin: units.gu(1)
        anchors.right: parent.right
        anchors.rightMargin: units.gu(1)

        //placeholderText: "Search"
        Keys.onEscapePressed: text = ""
        height: units.gu(4)

        primaryItem: AbstractButton {
            width: units.gu(3)
            Image {
                anchors.left: parent.left
                anchors.leftMargin: units.gu(0.5)
                anchors.verticalCenter: parent.verticalCenter
                source: "../assets/search.png"
            }
            onClicked: search.text = ""
        }
    }

    Column {
        id: buttons
        anchors.top: search.bottom
        anchors.topMargin: units.gu(1)
        anchors.left: parent.left
        anchors.right: parent.right

        // hide this on single panel version
        visible: !telephony.singlePane
        height: visible ? childrenRect.height : 0

        ListItem.ThinDivider {}

        ListItem.Standard {
            id: newMessage
            anchors.left: parent.left
            anchors.right: parent.right
            height: units.gu(4)

            __leftIconMargin: units.gu(2)
            __rightIconMargin: units.gu(2)

            icon: Qt.resolvedUrl("../assets/add_new_message_icon.png")
            iconFrame: false
            text: "New Message"
            onClicked: telephony.startNewMessage()
            selected: telephony.communication.loaded && telephony.view.newMessage
        }
    }

    ConversationProxyModel {
        id: conversationProxyModel
        conversationModel: conversationAggregatorModel
        searchString: search.text
        ascending: false
        grouped: true
        showLatestFromGroup: true
    }

    CommunicationsList {
        anchors.top: buttons.bottom
        anchors.bottom: keyboard.top
        anchors.left: parent.left
        anchors.right: parent.right
        // FIXME: reference to fake model needs to be removed before final release
        model: typeof(runtime) != "undefined" ? fakeMessages : conversationProxyModel
    }

    LocalWidgets.KeyboardRectangle {
        id: keyboard
    }
}
