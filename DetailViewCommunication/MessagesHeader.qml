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
import "../Widgets" as LocalWidgets

BaseMessageHeader {
    id: header

    property variant contact
    property string number
    property string title

    height: icon.height + units.gu(4)

    UbuntuShape {
        id: icon

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: units.gu(2)

        width: units.gu(6)
        height: units.gu(6)

        image: Image {
            source: contact && contact.avatar != "" ? contact.avatar : "../assets/avatar-default.png"
            fillMode: Image.PreserveAspectCrop
            asynchronous: true
        }
    }

    Label {
        id: name

        anchors.left: icon.right
        anchors.leftMargin: units.gu(2)
        anchors.right: parent.right
        anchors.rightMargin: units.gu(1)
        anchors.verticalCenter: parent.verticalCenter
        fontSize: "large"
        elide: Text.ElideRight
        color: "#333333"
        opacity: 0.6
        text: contact ? contact.displayLabel : title
    }

    /*BorderImage {
        id: separator
        anchors.right: contactDetailsButton.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: units.gu(2)
        source: "../assets/contacts_vertical_divider.sci"
        width: units.dp(2)
    }*/

    Button {
        id: contactDetailsButton
        ItemStyle.class: "transparent-button"
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: units.gu(2)
        width: units.gu(3)
        height: units.gu(3)

        iconSource: "../assets/contacts.png"
        onClicked: telephony.showContactDetails(contact, true)
        visible: contact != null
    }
}
