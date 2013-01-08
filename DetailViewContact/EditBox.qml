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
import Ubuntu.Components 0.1

Item {
    id: editBox

    property alias leftBox: leftBox
    property alias rightBox: rightBox

    width: units.gu(38)
    // center leftBox vertically
    height: leftBox.height + 2 * leftBox.anchors.topMargin

    BorderImage {
        id: background

        anchors.fill: parent
        source: "../assets/edit_contact_mode_box.png"
        border {left: units.dp(1); right: units.dp(1); top: units.dp(2); bottom: 0}
    }

    Item {
        id: leftBox

        anchors.left: parent.left
        anchors.right: separator.left
        anchors.top: parent.top
        anchors.topMargin: units.gu(1)
        height: childrenRect.height
    }

    Rectangle {
        id: separator

        anchors.right: rightBox.left
        width: units.dp(1)
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: "black"
        opacity: 0.1
    }

    Item {
        id: rightBox

        anchors.right: parent.right
        width: units.gu(11)
        anchors.top: parent.top
        anchors.bottom: parent.bottom
    }
}
