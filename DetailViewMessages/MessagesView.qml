import QtQuick 1.1
import TelephonyApp 0.1

Item {
    id: view

    property variant contact
    property string number
    property bool newMessage: false

    property string pendingMessage

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

    Component {
        id: newHeaderComponent
        NewMessageHeader {
            id: newHeader
            width: view.width
            height: 100

            onContactSelected: {
                view.contact = contact;
                view.number = number;
                view.newMessage = false;
            }

            onNumberSelected: {
                view.contact = null;
                view.number = number;
                view.newMessage = false;
            }
        }
    }

    Component {
        id: headerComponent
        MessagesHeader {
            id: header
            contact: view.contact
            number: view.number
            width: view.width
            height: 100
        }
    }

    Loader {
        id: headerLoader
        sourceComponent: view.newMessage ? newHeaderComponent : headerComponent
        anchors.top: parent.top
    }

     Loader {
         id: messagesLoader
         sourceComponent: newMessage ? undefined : messagesComponent
         anchors.top: headerLoader.bottom
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
        width: view.width
        height: 100
        visible: !view.newMessage
        onNewMessage: {
            if (messagesLoader.sourceComponent) {
                messagesLoader.item.addMessage(message)
            }

            if (chatManager.isChattingToContact(number)) {
                chatManager.sendMessage(number, message);
            } else {
                view.pendingMessage = message;
                chatManager.startChat(number);
            }
        }
        anchors.bottom: parent.bottom
    }
}

