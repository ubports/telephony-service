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

AbstractButton {
    id: button
    property alias text: label.text
    property alias icon: icon.source

    ItemStyle.class: "transparent"
    width: units.gu(6)
    height: units.gu(6)

    Image {
        id: icon
        anchors.top: parent.top
        anchors.topMargin: units.gu(1)
        anchors.horizontalCenter: parent.horizontalCenter
        height: units.gu(2)
        width: units.gu(2)
    }

    Label {
        id: label
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: units.gu(1)
        fontSize: "x-small"
        color: "#888888"
        width: paintedWidth
    }

    Rectangle {
        anchors.fill: parent
        color: "white"
        opacity: 0.4
        visible: button.pressed
    }
}
