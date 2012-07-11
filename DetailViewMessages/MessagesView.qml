import QtQuick 1.1
import QtMobility.contacts 1.1
import TelephonyApp 0.1
import "../"

Item {
    id: view

    property string viewName: "messages"
    property variant contact: contactWatcher.contact
    property string number
    property string contactId
    property bool newMessage: false
    property string threadId

    property string pendingMessage

    Binding { target: messageLogModel; property: "phoneNumber"; value: number; }
    Binding { target: messageLogModel; property: "threadId"; value: threadId; }

    Binding { target: contactWatcher; property: "number"; value: number; }
    Binding { target: contactWatcher; property: "contactId"; value: contactId; }

    function refreshModel() {
        messageLogModel.refreshModel()
    }

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
    }

    // make sure the text channel gets closed after chatting
    Component.onDestruction: chatManager.endChat(number);

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
                contactWatcher.contact = contact;
                view.newMessage = false;
                view.threadId = ""
                refreshModel()
            }

            onNumberSelected: {
                view.number = number;
                view.contactId = "";
                view.newMessage = false;
                view.threadId = ""
                refreshModel()
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
                refreshModel()
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

