import QtQuick 1.1
import "../Widgets"

AbstractButton {
    // FIXME: maybe create a Line component to make the code more readable
    Rectangle {
        height: 1
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: "#9b9b9b"
    }

    Image {
        id: contactPhotoItem
        source: avatar != "" ? avatar : "../assets/default_avatar.png"
        width: 35
        height: width
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.verticalCenter: parent.verticalCenter
    }

    Item {
        anchors.left: contactPhotoItem.right
        anchors.leftMargin: 6
        anchors.right: parent.right
        anchors.rightMargin: 2
        anchors.top: parent.top
        anchors.topMargin: 4
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 6

        TextCustom {
            id: contactNameItem
            anchors.left: parent.left
            anchors.right: parent.right
            fontSize: "medium"
            text: contactAlias
        }

        TextCustom {
            id: lastMessageDateItem
            anchors.right: parent.right
            color: "#828282"
            fontSize: "small"
            text: Qt.formatDateTime(timestamp, Qt.DefaultLocaleShortDate)
        }

        TextCustom {
            id: lastMessageItem
            anchors.top: contactNameItem.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: 15
            fontSize: "small"
            height: 30
            text: text
            elide: Text.ElideRight
        }
    }
}
