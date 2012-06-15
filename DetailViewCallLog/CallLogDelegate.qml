import QtQuick 1.1
import "../Widgets"

Rectangle {
    id: callItem
    height: 64
    border.color: "black"
    border.width: 1

    signal clicked(string contactId)
    signal actionClicked(string contactId, string phoneNumber)

    Rectangle {
        height: 1
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: "black"
    }

    Image {
        id: photoItem
        source: avatar
        width: 56
        height: width
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.verticalCenter: parent.verticalCenter
    }

    TextCustom {
        id: displayNameItem
        anchors.top: parent.top
        anchors.topMargin: 1
        anchors.left: photoItem.right
        anchors.right: directionItem.left
        anchors.leftMargin: 2
        fontSize: "medium"
        text: contactAlias
    }

    TextCustom {
        id: phoneTypeItem
        anchors.top: displayNameItem.bottom
        anchors.topMargin: 2
        anchors.left: photoItem.right
        anchors.leftMargin: 2
        fontSize: "small"
        text: phoneType
    }

    TextCustom {
        id: phoneItem
        anchors.top: displayNameItem.bottom
        anchors.topMargin: 2
        anchors.left: phoneTypeItem.right
        anchors.leftMargin: 4
        fontSize: "small"
        text: phoneNumber
        transformOrigin: Item.Center
    }

    TextCustom {
        id: dateItem
        anchors.top: phoneTypeItem.bottom
        anchors.topMargin: 2
        anchors.left: photoItem.right
        anchors.leftMargin: 2
        fontSize: "small"
        text: Qt.formatDateTime(timestamp, Qt.DefaultLocaleLongDate)
    }

    Image {
        id: directionItem
        source: {
            if(incoming) {
                if(missed) {
                    "../assets/icon_missed_call.png"
                } else {
                    "../assets/icon_incoming_call.png"
                }
            } else {
                "../assets/icon_outgoing_call.png"
            }
        }
        width: height
        height: dateItem.height
        anchors.leftMargin: 1
        anchors.left: dateItem.right
        anchors.verticalCenter: dateItem.verticalCenter
    }

    Rectangle {
        id: actionBox
        border.color: "black"
        width: height
        height: parent.height
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        Button {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 5
            height: width

            iconSource: "../assets/call_icon.png"

            onClicked: callItem.actionClicked(contactId, phoneNumber)
        }
    }

    MouseArea {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.right: actionBox.left

        onClicked: callItem.clicked(contactId)
    }
}
