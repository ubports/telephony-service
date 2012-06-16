import QtQuick 1.1
import TelephonyApp 0.1
import "../Widgets"

Item {
    id: messageList

    SearchEntry {
        id: search

        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10

        leftIconSource: text ? "../assets/quick_search_delete_icon.png" : "../assets/search_icon.png"
        hint: "Search messages"
        onLeftIconClicked: text = ""
    }

    ListItem {
        id: newMessage

        anchors.top: search.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.right: parent.right

        topSeparator: true
        isIcon: true
        iconSource: "../assets/icon_message_grey.png"
        text: "New Message"
        // FIXME: maybe use a signal and handle in the instance
        onClicked: telephony.startNewMessage()
        selected: telephony.messages.loaded && telephony.view.newMessage
    }

    MessagesProxyModel {
        id: messagesProxyModel
        messagesModel: conversationLogModel
        ascending: false
    }

    MessagesList {
        anchors.top: newMessage.bottom
        // FIXME: make that filling in the available space generic
        height: parent.height - search.height - newMessage.height - 10
        anchors.left: parent.left
        anchors.right: parent.right
        // FIXME: reference to fake model needs to be removed before final release
        model: typeof(runtime) != "undefined" ? fakeMessages : messagesProxyModel
        filter: search.searchQuery
    }
}
