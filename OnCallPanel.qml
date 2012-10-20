import QtQuick 2.0
import TelephonyApp 0.1
import "Widgets" as LocalWidgets
import Ubuntu.Components 0.1

AbstractButton {
    id: onCallPanel

    property QtObject call: callManager.foregroundCall

    height: units.dp(61)

    BorderImage {
        id: background

        source: "assets/oncall_background.png"
        horizontalTileMode: BorderImage.Repeat
        verticalTileMode: BorderImage.Repeat
        border {top: units.dp(5); bottom: 0; left: 0; right: 0}
    }


    Item {
        id: avatar

        anchors.top: parent.top
        anchors.topMargin: units.dp(13)
        anchors.left: parent.left
        anchors.leftMargin: units.dp(9)
        width: units.dp(38)
        height: units.dp(38)

        LocalWidgets.FramedImage {
            id: avatarIcon

            anchors.fill: parent
            anchors.margins: units.dp(1)
            source: call ? call.contactAvatar : fallbackSource
            fallbackSource: "assets/avatar_contacts_list.png"
            frameSource: "assets/oncall_picture_frame.sci"
        }

        Image {
            id: avatarEmblem

            source: "assets/oncall_phone_icon.png"
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: units.dp(-4)
            anchors.leftMargin: units.dp(-6)
        }
    }

    Column {
        anchors.left: avatar.right
        anchors.leftMargin: units.dp(8)
        anchors.right: parent.right
        anchors.rightMargin: units.dp(5)
        anchors.verticalCenter: avatar.verticalCenter

        TextCustom {
            anchors.left: parent.left
            anchors.right: parent.right

            text: "On Call"
            fontSize: "small"
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
            fontSize: "large"
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
        }
    }

    Item {
        id: duration

        visible: stopWatch.time > 0
        anchors.top: parent.top
        anchors.topMargin: units.dp(19)
        anchors.right: parent.right
        anchors.rightMargin: units.dp(10)
        height: units.dp(24)
        width: stopWatch.width

        BorderImage {
            id: durationBackground

            source: "assets/oncall_call_duration_frame.png"
            anchors.fill: parent
            horizontalTileMode: BorderImage.Stretch
            verticalTileMode: BorderImage.Stretch
            border {left: units.dp(10); right: units.dp(10); top: units.dp(12); bottom: units.dp(12)}
        }

        LocalWidgets.StopWatch {
            id: stopWatch

            anchors.centerIn: parent
            fontSize: "small"
            color: "#5d960f"
            time: call ? call.elapsedTime : 0
        }
    }
}
