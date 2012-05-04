import QtQuick 1.1
import "../Widgets"

AbstractButton {
    id: newMessage

    Item {
        anchors.fill: parent
        anchors.leftMargin: 5
        anchors.rightMargin: 5

        Row {
            spacing: 6
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height

            Item {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 35

                Image {
                    fillMode: Image.PreserveAspectFit
                    anchors.fill: parent
                    anchors.margins: 3
                    source: "../assets/icon_message_grey.png" // FIXME: wrong icon
                }
            }

            TextCustom {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                text: "New Message"
                verticalAlignment: Text.AlignVCenter
            }
        }

        Rectangle {
            height: 1
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            color: "#9b9b9b"
        }
    }
}
