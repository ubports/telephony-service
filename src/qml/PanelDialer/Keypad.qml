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

Item {
    id: keypad

    property int fontPixelSize: units.dp(43)
    property int keysWidth: units.gu(11)
    property int keysHeight: units.gu(8)

    width: keys.width
    height: keys.height

    signal keyPressed(int keycode, string label)

    Grid {
        id: keys

        rows: 4
        columns: 3
        spacing: units.gu(1)

        KeypadButton {
            width: keysWidth
            height: keysHeight
            labelFontPixelSize: fontPixelSize
            label: "1"
            keycode: Qt.Key_1
            onClicked: keypad.keyPressed(keycode, label)
            onPressAndHold: mainView.callVoicemail()
            iconSource: "../assets/voicemail.png"
        }

        KeypadButton {
            width: keysWidth
            height: keysHeight
            labelFontPixelSize: fontPixelSize
            label: "2"
            sublabel: "ABC"
            keycode: Qt.Key_2
            onClicked: keypad.keyPressed(keycode, label)
        }

        KeypadButton {
            width: keysWidth
            height: keysHeight
            labelFontPixelSize: fontPixelSize
            label: "3"
            sublabel: "DEF"
            keycode: Qt.Key_3
            onClicked: keypad.keyPressed(keycode, label)
        }

        KeypadButton {
            width: keysWidth
            height: keysHeight
            labelFontPixelSize: fontPixelSize
            label: "4"
            sublabel: "GHI"
            keycode: Qt.Key_4
            onClicked: keypad.keyPressed(keycode, label)
        }

        KeypadButton {
            width: keysWidth
            height: keysHeight
            labelFontPixelSize: fontPixelSize
            label: "5"
            sublabel: "JKL"
            keycode: Qt.Key_5
            onClicked: keypad.keyPressed(keycode, label)
        }

        KeypadButton {
            width: keysWidth
            height: keysHeight
            labelFontPixelSize: fontPixelSize
            label: "6"
            sublabel: "MNO"
            keycode: Qt.Key_6
            onClicked: keypad.keyPressed(keycode, label)
        }

        KeypadButton {
            width: keysWidth
            height: keysHeight
            labelFontPixelSize: fontPixelSize
            label: "7"
            sublabel: "PQRS"
            keycode: Qt.Key_7
            onClicked: keypad.keyPressed(keycode, label)
        }

        KeypadButton {
            width: keysWidth
            height: keysHeight
            labelFontPixelSize: fontPixelSize
            label: "8"
            sublabel: "TUV"
            keycode: Qt.Key_8
            onClicked: keypad.keyPressed(keycode, label)
        }

        KeypadButton {
            width: keysWidth
            height: keysHeight
            labelFontPixelSize: fontPixelSize
            label: "9"
            sublabel: "WXYZ"
            keycode: Qt.Key_9
            onClicked: keypad.keyPressed(keycode, label)
        }

        KeypadButton {
            width: keysWidth
            height: keysHeight
            labelFontPixelSize: fontPixelSize
            isCorner: true
            corner: Qt.BottomLeftCorner
            label: "*"
            keycode: Qt.Key_Asterisk
            onClicked: keypad.keyPressed(keycode, label)
        }

        KeypadButton {
            width: keysWidth
            height: keysHeight
            labelFontPixelSize: fontPixelSize
            label: "0"
            sublabel: "+"
            sublabelSize: "medium"
            keycode: Qt.Key_0
            onClicked: keypad.keyPressed(keycode, label)
            onPressAndHold: keypad.keyPressed(keycode, sublabel)
        }

        KeypadButton {
            width: keysWidth
            height: keysHeight
            labelFontPixelSize: fontPixelSize
            isCorner: true
            corner: Qt.BottomRightCorner
            label: "#"
            keycode: Qt.Key_ssharp
            onClicked: keypad.keyPressed(keycode, label)
        }
    }
}
