import QtQuick 1.1
import TelephonyApp 0.1
import "Widgets"

Rectangle {
    id: onCallPanel

    property QtObject call: callManager.foregroundCall
    signal clicked()

    height: 64
    border.color: "black"
    color: "white"
    z: 1

    Image {
        id: avatar
        anchors.margins: 5
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: height
        source: (call && call.contactAvatar != "") ? call.contactAvatar : "assets/default_avatar.png"
    }

    Text {
        text: {
            if (call) {
                if (call.voicemail) {
                    "Voicemail"
                } else {
                    call.phoneNumber
                }
            } else {
                ""
            }
        }
        anchors.margins: 5
        anchors.left: avatar.right
        anchors.verticalCenter: avatar.verticalCenter
    }

    MouseArea {
        anchors.fill: parent
        onClicked: onCallPanel.clicked();
    }
}
