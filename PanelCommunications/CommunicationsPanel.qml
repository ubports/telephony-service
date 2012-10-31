import QtQuick 2.0
import TelephonyApp 0.1
import "../Widgets" as LocalWidgets
import Ubuntu.Components.ListItems 0.1 as ListItem
import Ubuntu.Components 0.1

LocalWidgets.TelephonyPage {
    id: messageList
    title: "Conversations"

    anchors.fill: parent

    TextField {
        id: search

        anchors.top: parent.top
        anchors.topMargin: units.gu(1)
        anchors.left: parent.left
        anchors.leftMargin: units.gu(1)
        anchors.right: parent.right
        anchors.rightMargin: units.gu(1)

        placeholderText: "Search messages"
        Keys.onEscapePressed: text = ""

        primaryItem: AbstractButton {
            width: units.gu(3)
            Image {
                anchors.left: parent.left
                anchors.leftMargin: units.gu(0.5)
                anchors.verticalCenter: parent.verticalCenter
                source: search.text ? "../assets/quick_search_delete_icon.png" : "../assets/search_icon.png"
            }
            onClicked: search.text = ""
        }
    }

    Column {
        id: buttons
        anchors.top: search.bottom
        anchors.topMargin: units.gu(1)
        anchors.left: parent.left
        anchors.right: parent.right

        ListItem.ThinDivider {}

        ListItem.Standard {
            id: newMessage
            anchors.left: parent.left
            anchors.right: parent.right
            height: units.gu(4)
            __leftIconMargin: units.gu(2)
            __rightIconMargin: units.gu(2)

            icon: Qt.resolvedUrl("../assets/add_new_message_icon.png")
            iconFrame: false
            text: "New Message"
            // FIXME: maybe use a signal and handle in the instance
            onClicked: telephony.startNewMessage()
            selected: telephony.communication.loaded && telephony.view.newMessage
        }
    }

    ConversationProxyModel {
        id: conversationProxyModel
        conversationModel: conversationAggregatorModel
        searchString: search.text
        ascending: false
        grouped: true
    }

    CommunicationsList {
        anchors.top: buttons.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        // FIXME: reference to fake model needs to be removed before final release
        model: typeof(runtime) != "undefined" ? fakeMessages : conversationProxyModel
    }
}
