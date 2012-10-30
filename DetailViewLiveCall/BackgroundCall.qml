import QtQuick 2.0
import Ubuntu.Components 0.1

Rectangle {
    property QtObject call

    border.color: "black"
    color: "white"
    height: units.gu(8)

    Image {
        id: avatar
        anchors.margins: units.gu(0.5)
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: height
        source: (call && call.contactAvatar != "") ? call.contactAvatar : "../assets/avatar_incall_rightpane.png"
        onStatusChanged: if (status == Image.Error) source = "../assets/avatar_incall_rightpane.png"
        asynchronous: true
    }

    Text {
        text: call ? call.phoneNumber : ""
        anchors.margins: units.gu(0.5)
        anchors.left: avatar.right
        anchors.verticalCenter: avatar.verticalCenter
    }

    Button {
        id: swapButton

        anchors.margins: units.gu(0.5)
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        text: "Swap Calls"

        onClicked: call.held = false
    }
}
