import QtQuick 1.1
import Ubuntu.Components 0.1

Page {
    anchors.fill: parent
    FocusScope {
        property string viewName: "keypad"

        anchors.fill: parent

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
