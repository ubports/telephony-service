import QtQuick 1.1

Item {
    id: messageList

    property variant model: messages

    Column {
        anchors.fill: parent

        MessagesSearchEntry {
            id: search

            height: 40
            anchors.left: parent.left
            anchors.right: parent.right
        }

        NewMessageButton {
            id: newMessage

            height: 45
            anchors.left: parent.left
            anchors.right: parent.right
        }

        MessagesList {
            height: parent.height - search.height - newMessage.height // FIXME: make that filling in the available space generic
            anchors.left: parent.left
            anchors.right: parent.right
            model: messageList.model
        }
    }
}
