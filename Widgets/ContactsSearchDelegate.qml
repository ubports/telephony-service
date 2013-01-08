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

Item {
    id: delegate

    property color fontColor: Qt.rgba(0.4, 0.4, 0.4, 1.0)
    property string fontSize: "medium"
    property bool isLast: false

    signal contactClicked(variant contact, string number)

    height: childrenRect.height

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: childrenRect.height

        // show one contact entry per phone number in the details
        Repeater {
            model: contact.phoneNumbers

            Item {
                anchors.left: parent.left
                anchors.right: parent.right
                height: units.gu(4)

                BorderImage {
                    id: background

                    anchors.fill: parent
                    anchors.bottomMargin: delegate.isLast ? -units.gu(0.5) : 0
                    source: delegate.isLast ? "../assets/input_field_autofill_bottom.sci" : "../assets/input_field_autofill_middle.sci"
                }

                Item {
                    id: labels

                    anchors.left: parent.left
                    anchors.leftMargin: units.gu(2)
                    anchors.right: parent.right
                    anchors.rightMargin: units.gu(2)
                    anchors.verticalCenter: parent.verticalCenter
                    height: childrenRect.height

                    Label {
                        id: contactName

                        anchors.left: parent.left
                        text: contact.displayLabel
                        fontSize: delegate.fontSize
                        color: fontColor
                        style: Text.Raised
                        styleColor: "white"
                    }

                    Label {
                        anchors.left: contactName.right
                        anchors.leftMargin: units.gu(1)
                        anchors.right: parent.right
                        horizontalAlignment: Text.AlignRight
                        elide: Text.ElideRight
                        text: modelData.number
                        color: "dark gray"
                        fontSize: delegate.fontSize
                        style: Text.Raised
                        styleColor: "white"
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: delegate.contactClicked(contact, modelData.number)
                }
            } // Rectangle
        } // Repeater
    } // Column
}
