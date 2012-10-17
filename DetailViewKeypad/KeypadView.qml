import QtQuick 2.0
import "../Widgets" as LocalWidgets

LocalWidgets.TelephonyPage {
    title: "Keypad"
    anchors.fill: parent
    FocusScope {
        anchors.fill: parent
        focus: true

        Rectangle {
            id: background

            anchors.fill: parent
            color: "#3a3c41"
        }

        Item {
            width: keypad.width
            height: childrenRect.height

            anchors.centerIn: parent
            anchors.verticalCenterOffset: -26

            KeypadEntry {
                id: keypadEntry

                anchors.left: keypad.left
                anchors.right: keypad.right
                anchors.leftMargin: -2
                anchors.rightMargin: -2
                focus: true
                Keys.forwardTo: [callButton]
            }

            Keypad {
                id: keypad

                anchors.top: keypadEntry.bottom
                onKeyPressed: keypadEntry.value += label
            }

            CallButton {
                id: callButton

                anchors.top: keypad.bottom
                anchors.topMargin: 16
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: telephony.callNumber(keypadEntry.value)
            }
        }
    }
}
