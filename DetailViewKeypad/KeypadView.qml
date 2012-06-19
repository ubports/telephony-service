import QtQuick 1.1

Item {
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
        }

        Keypad {
            id: keypad

            anchors.top: keypadEntry.bottom
            onKeyPressed: keypadEntry.value += label
        }

        CallButton {
            anchors.top: keypad.bottom
            anchors.topMargin: 16
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: telephony.callNumber(keypadEntry.value)
        }
    }
}
