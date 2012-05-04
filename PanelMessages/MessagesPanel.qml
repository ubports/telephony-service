import QtQuick 1.1
import "../Widgets"

Item {
    id: messageList

    Column {
        anchors.fill: parent
        anchors.margins: 5
        spacing: 5

        SearchEntry {
            id: search
            height: 30
            anchors.left: parent.left
            anchors.right: parent.right

            leftIconSource: text ? "../assets/cross.png" : "../assets/search_icon.png"
            onLeftIconClicked: text = ""
        }

        Button {
            id: newMessage
            height: 45
            anchors.left: parent.left
            anchors.right: parent.right

            icon: "../assets/icon_message_grey.png"
            iconWidth: 35
            text: "New Message"

            onClicked: telephony.startNewMessage()
        }

        MessagesList {
            height: parent.height - search.height - newMessage.height - 10 // FIXME: make that filling in the available space generic
            anchors.left: parent.left
            anchors.right: parent.right
            model: messages
            filter: search.searchQuery
        }
    }
}
