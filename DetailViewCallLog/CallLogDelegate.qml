import QtQuick 1.1
import "../Widgets"

Rectangle {
    id: callItem
    height: 64
    width: parent.width

    signal clicked(string contactId)

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
        anchors.right: directionItem.left
        fontSize: "small"
        text: timestamp
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
        width: 48
        height: 48
        anchors.rightMargin: 1
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
    }

    MouseArea {
        anchors.fill: parent
        onClicked: callItem.clicked(contactId)
    }
}
