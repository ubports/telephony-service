import QtQuick 1.1
import TelephonyApp 0.1
import "../Widgets"
import Ubuntu.Components.ListItems 0.1 as ListItem

Item {
    id: messageList

    anchors.fill: parent

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

    ListItem.Standard {
        id: newMessage

        anchors.top: search.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.right: parent.right

        control: PanelButton {
            iconSource: "../assets/add_new_message_icon.png"
            text: "New Message"
            // FIXME: maybe use a signal and handle in the instance
            onClicked: telephony.startNewMessage()
            selected: telephony.messages.loaded && telephony.view.newMessage
        }
    }

    MessagesProxyModel {
        id: messagesProxyModel
        messagesModel: messageLogModel
        searchString: search.text
        ascending: false
        onlyLatest: true
    }

    MessagesList {
        anchors.top: newMessage.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        // FIXME: reference to fake model needs to be removed before final release
        model: typeof(runtime) != "undefined" ? fakeMessages : messagesProxyModel
    }
}
