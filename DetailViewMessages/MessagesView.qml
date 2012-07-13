import QtQuick 1.1
import QtMobility.contacts 1.1
import TelephonyApp 0.1
import "../"

Item {
    id: view

    property string viewName: "messages"
    property alias contact: contactWatcher.contact
    property alias number: contactWatcher.phoneNumber
    property alias customId: contactWatcher.customId
    property bool newMessage: false
    property string threadId

    property string pendingMessage

    ContactWatcher {
        id: contactWatcher
    }

    Connections {
        target: chatManager

        onChatReady: {
            if (contactId != number) {
                return;
            }

            if (pendingMessage != "") {
                chatManager.sendMessage(number, pendingMessage);
                pendingMessage = "";
            }
        }

        onMessageReceived: {
            if (contactModel.comparePhoneNumbers(contactId, number)) {
                // if the message received is in the current view, mark it as read
                chatManager.acknowledgeMessages(contactId);
            }
        }
    }

    // make sure the text channel gets closed after chatting
    Component.onDestruction: chatManager.endChat(number);

    onNumberChanged: {
        // get the contact
        view.contact = contactModel.contactFromPhoneNumber(number);

        if (number != "") {
            // and mark messages that came from the telepathy text channel as read
            chatManager.acknowledgeMessages(number);
        }

    }

    Item {
        id: background

        anchors.fill: parent

        Image {
            anchors.fill: parent
            source: "../assets/noise_tile.png"
            fillMode: Image.Tile
        }

        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.05
        }
    }

    Component {
        id: newHeaderComponent

        NewMessageHeader {
            width: view.width

            onContactSelected: {
                view.number = number;
                view.newMessage = false;
                view.threadId = ""
            }

            onNumberSelected: {
                view.number = number;
                view.newMessage = false;
                view.threadId = ""
            }
        }
    }

    Component {
        id: headerComponent

        MessagesHeader {
            width: view.width
            contact: view.contact
            number: view.number
        }
    }

    Loader {
        id: headerLoader

        sourceComponent: view.newMessage ? newHeaderComponent : headerComponent
        anchors.top: parent.top
    }

    Image {
        anchors.top: messagesLoader.top
        anchors.bottom: footer.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        source: "../assets/right_pane_pattern.png"
        fillMode: Image.Tile
    }

    Loader {
        id: messagesLoader

        sourceComponent: view.newMessage ? undefined : messagesComponent
        anchors.top: headerLoader.bottom
        anchors.bottom: footer.top
    }

    Component {
        id: messagesComponent
        Messages {
            id: messages
            width: view.width
            height: view.height - footer.height - headerLoader.height
            threadId: view.threadId
            number: view.number
        }
    }

    MessagesFooter {
        id: footer

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        onNewMessage: {
            // if the user didn't select a number from the new message header, just
            // use whatever is on the text field
            if (view.newMessage) {
                var phoneNumber = headerLoader.item.text;
                view.number = phoneNumber
                view.newMessage = false;
                view.threadId = ""
            }

            if (chatManager.isChattingToContact(number)) {
                chatManager.sendMessage(number, message);
            } else {
                view.pendingMessage = message;
                chatManager.startChat(number);
            }
        }
    }
}

