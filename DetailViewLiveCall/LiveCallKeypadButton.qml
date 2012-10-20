/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
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

    property alias iconSource: icon.source
    property bool selected: false
    property int corner

    width: units.dp(64)
    height: units.dp(48)

    BorderImage {
        anchors.fill: parent
        visible: button.state == "pressed"
        source: {
            switch (button.corner) {
            case Qt.TopLeftCorner:
                return "../assets/keypad_select_top_left.png"
            case Qt.TopRightCorner:
                return "../assets/keypad_select_top_right.png"
            case Qt.BottomLeftCorner:
                return "../assets/keypad_select_bottom_left.png"
            case Qt.BottomRightCorner:
                return "../assets/keypad_select_bottom_right.png"
            default:
                return ""
            }
        }
        border {right: units.dp(14); left: units.dp(14); top: units.dp(10); bottom: units.dp(10)}
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
    }

    Image {
        id: icon

        anchors.centerIn: parent
        anchors.horizontalCenterOffset: units.dp(1)
        fillMode: Image.PreserveAspectFit
    }
}
