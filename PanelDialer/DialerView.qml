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

LocalWidgets.TelephonyPage {
    title: "Call"
    anchors.fill: parent
    property string voicemailNumber: callManager.voicemailNumber
    property alias dialNumber: keypadEntry.value
    property alias input: keypadEntry.input

    function isVoicemailActive() {
        return telephony.isVoicemailActive();
    }

    tools: ToolbarActions {
        active: false
        lock: true
    }

    FocusScope {
        anchors.fill: parent
        focus: true

        KeypadEntry {
            id: keypadEntry

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            focus: true
            placeHolder: "Enter a number"
            Keys.forwardTo: [callButton]
        }

        Keypad {
            id: keypad

            anchors.top: keypadEntry.bottom
            onKeyPressed: {
                if (input.cursorPosition != 0)  {
                    var position = input.cursorPosition;
                    input.text = input.text.slice(0, input.cursorPosition) + label + input.text.slice(input.cursorPosition);
                    input.cursorPosition = position +1 ;
                } else {
                    keypadEntry.value += label
                }
            }
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: units.gu(3)
        }

        Item {
            id: footer

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: units.gu(12)

            BorderImage {
                id: divider3

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                source: "../assets/horizontal_divider.sci"
            }

            CallButton {
                id: callButton
                objectName: "callButton"
                anchors.top: divider3.bottom
                anchors.topMargin: units.gu(2)
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: telephony.callNumber(keypadEntry.value)
                enabled: dialNumber != ""
            }

            CustomButton {
                id: contactListButton
                objectName: "contactListButton"
                anchors.right: callButton.left
                anchors.verticalCenter: callButton.verticalCenter
                anchors.rightMargin: units.gu(1)
                icon: "../assets/contacts.png"
                iconWidth: units.gu(3)
                iconHeight: units.gu(3)
                width: units.gu(7)
                height: units.gu(7)
                onClicked: {
                    telephony.switchToTab(telephony.contactDetails.tab)
                }
            }

            CustomButton {
                id: backspace
                objectName: "eraseButton"
                anchors.left: callButton.right
                anchors.verticalCenter: callButton.verticalCenter
                anchors.leftMargin: units.gu(1)
                width: units.gu(7)
                height: units.gu(7)
                icon: "../assets/erase.png"
                iconWidth: units.gu(3)
                iconHeight: units.gu(3)

                onPressAndHold: input.text = ""

                onClicked:  {
                    if (input.cursorPosition != 0)  {
                        var position = input.cursorPosition;
                        input.text = input.text.slice(0, input.cursorPosition - 1) + input.text.slice(input.cursorPosition);
                        input.cursorPosition = position - 1;
                    }
                }
            }
        }
    }
}
