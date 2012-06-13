import QtQuick 1.1
import "../Widgets"

AbstractButton {
    property variant contact: fakeContacts.fromId(contactId)

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
        // FIXME: the dummydata prefix should be in the model
        source: "../dummydata/" + contact.photo
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
            text: contact.displayName
        }

        TextCustom {
            id: lastMessageDateItem
            anchors.right: parent.right
            color: "#828282"
            fontSize: "small"
            text: lastMessageDate
        }

        TextCustom {
            id: lastMessageItem
            anchors.top: contactNameItem.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: 15
            fontSize: "small"
            height: 30
            text: lastMessageContent
            elide: Text.ElideRight
        }
    }
}
