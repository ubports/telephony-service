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
    id: header
    property string text
    height: childrenRect.height
    clip: true

    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
    }

    Column {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: childrenRect.height

        Item {
            id: headerLabel
            height: units.gu(6)
            anchors {
                left: parent.left
                right: parent.right
            }

            Label {
                text: header.text
                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    leftMargin: units.gu(2)
                }
                fontSize: "x-large"
                font.weight: Font.Light
                color: "#333333"
                opacity: 0.4
                height: units.gu(4)
                verticalAlignment: Text.AlignVCenter
            }
        }

        BorderImage {
            id: divider
            anchors.left: parent.left
            anchors.right: parent.right
            source: "../assets/PageHeaderBaseDivider.sci"
        }
    }
}
