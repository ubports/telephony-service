import QtQuick 1.1
import QtMobility.contacts 1.1
import TelephonyApp 0.1
import "../ContactUtils"

Item {
    id: view

    property string viewName: "messages"
    property variant contact
    property string number
    property bool newMessage: false

    property string pendingMessage

    function addMessage(message) {
        messagesLoader.item.addMessage(message, false)
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

    Component.onCompleted: messageLogModel.phoneNumber = number;

    // make sure the text channel gets closed after chatting
    Component.onDestruction: chatManager.endChat(number);

    onNumberChanged: messageLogModel.phoneNumber = number;

    ContactLoader {
        id: contactLoader

        filter: DetailFilter {
            detail: ContactDetail.PhoneNumber
            field: PhoneNumber.number
            value: view.number
            matchFlags: DetailFilter.MatchPhoneNumber
        }

        onContactLoaded: view.contact = contact
    }

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

