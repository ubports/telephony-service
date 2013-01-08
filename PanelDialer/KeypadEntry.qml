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

FocusScope {
    id: keypadEntry

    property alias value: input.text
    property alias input: input
    property alias placeHolder: hint.text
    property alias placeHolderPixelFontSize: hint.font.pixelSize

    height: units.gu(8)

    Label {
        id: dots
        clip: true
        width: input.contentWidth > input.width ? contentWidth : 0
        visible: input.contentWidth > input.width
        anchors.left: parent.left
        text: "..."
        font.pixelSize: units.dp(43)
        font.weight: Font.Light
        font.family: "Ubuntu"
        color: "#464646"
    }

    TextInput {
        id: input

        anchors.left: dots.right
        anchors.right: parent.right
        anchors.rightMargin: units.gu(2)
        anchors.verticalCenter: parent.verticalCenter
        horizontalAlignment: TextInput.AlignRight
        text: ""
        font.pixelSize: units.dp(43)
        font.weight: Font.Light
        font.family: "Ubuntu"
        color: "#464646"
        focus: true
        cursorVisible: true
        clip: true
        opacity: 0.9

        // force cursor to be always visible
        onCursorVisibleChanged: {
            if (!cursorVisible)
                cursorVisible = true
        }
    }

    MouseArea {
        anchors.fill: input
        property bool held: false
        onClicked: {
            input.cursorPosition = input.positionAt(mouseX,TextInput.CursorOnCharacter)
        }
        onPressAndHold: {
            if (input.text != "") {
                held = true
                input.selectAll()
                input.copy()
            } else {
                input.paste()
            }
        }
        onReleased: {
            if(held) {
                input.deselect()
                held = false
            }

        }
    }

    Label {
        id: hint
        visible: input.text == ""
        anchors.centerIn: input
        text: ""
        font.pixelSize: units.dp(20)
        font.weight: Font.Light
        font.family: "Ubuntu"
        color: "#464646"
        opacity: 0.9
    }

    Image {
        id: divider

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        source: "../assets/dialer_top_number_bg.png"
    }

}
