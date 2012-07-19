import QtQuick 1.1
import TelephonyApp 0.1
import "Widgets"

AbstractButton {
    id: onCallPanel

    property QtObject call: callManager.foregroundCall

    height: 61

    BorderImage {
        id: background

        source: "assets/oncall_background.png"
        horizontalTileMode: BorderImage.Repeat
        verticalTileMode: BorderImage.Repeat
        border {top: 5; bottom: 0; left: 0; right: 0}
    }


    Item {
        id: avatar

        anchors.top: parent.top
        anchors.topMargin: 13
        anchors.left: parent.left
        anchors.leftMargin: 9
        width: 38
        height: 38

        Image {
            id: avatarIcon

            anchors.fill: avatarFrame
            anchors.margins: 1
            fillMode: Image.PreserveAspectCrop
            smooth: true
            source: (call && call.contactAvatar != "") ? call.contactAvatar : "assets/avatar_contacts_list.png"
            onStatusChanged: if (status == Image.Error) source = "assets/avatar_contacts_list.png"
            asynchronous: true
        }

        BorderImage {
            id: avatarFrame

            source: "assets/oncall_picture_frame.png"

            border {top: 3; bottom: 1; left: 2; right: 2}
            horizontalTileMode: BorderImage.Stretch
            verticalTileMode: BorderImage.Stretch
        }

        Image {
            id: avatarEmblem

            source: "assets/oncall_phone_icon.png"
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: -4
            anchors.leftMargin: -6
        }
    }

    Column {
        anchors.left: avatar.right
        anchors.leftMargin: 8
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.verticalCenter: avatar.verticalCenter

        TextCustom {
            anchors.left: parent.left
            anchors.right: parent.right

            text: "On Call"
            fontSize: "medium"
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
        }

        TextCustom {
            anchors.left: parent.left
            anchors.right: parent.right

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
            fontSize: "x-large"
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
        }
    }

    Item {
        id: duration

        // FIXME: the visuals are correct but the backend is not yet connected
        visible: false
        anchors.top: parent.top
        anchors.topMargin: 19
        anchors.right: parent.right
        anchors.rightMargin: 10
        height: 24
        width: durationLabel.width + 14*2

        BorderImage {
            id: durationBackground

            source: "assets/oncall_call_duration_frame.png"
            anchors.fill: parent
            horizontalTileMode: BorderImage.Stretch
            verticalTileMode: BorderImage.Stretch
            border {left: 10; right: 10; top: 12; bottom: 12}
        }

        TextCustom {
            id: durationLabel

            anchors.centerIn: parent
            fontSize: "medium"
            elide: Text.ElideRight
            color: "#5d960f"
            text: "00:21"
        }
    }
}
