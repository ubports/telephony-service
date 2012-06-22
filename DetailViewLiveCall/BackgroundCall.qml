import QtQuick 1.1
import "../Widgets"

Rectangle {
    property QtObject call

    border.color: "black"
    color: "white"
    height: 64

    Image {
        id: avatar
        anchors.margins: 5
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: height
        source: (call && call.contactAvatar != "") ? call.contactAvatar : "../assets/default_avatar.png"
    }

    Text {
        text: call ? call.phoneNumber : ""
        anchors.margins: 5
        anchors.left: avatar.right
        anchors.verticalCenter: avatar.verticalCenter
    }

    Button {
        id: swapButton

        anchors.margins: 5
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        text: "Swap Calls"
    }
}
