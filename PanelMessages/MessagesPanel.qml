import QtQuick 1.1
import "../Widgets"
// FIXME: the Column can be the top item
Item {
    id: messageList

    Column {
        // FIXME: the anchors should be set in the instance
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

            iconSource: "../assets/icon_message_grey.png"
            text: "New Message"
            // FIXME: maybe use a signal and handle in the instance
            onClicked: telephony.startNewMessage()
        }

        MessagesList {
            // FIXME: make that filling in the available space generic
            height: parent.height - search.height - newMessage.height - 10
            anchors.left: parent.left
            anchors.right: parent.right
            // FIXME: reference to fake model needs to be removed before final release
            model: fakeMessages
            filter: search.searchQuery
        }
    }
}
