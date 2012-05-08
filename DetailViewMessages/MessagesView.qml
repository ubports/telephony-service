import QtQuick 1.1

Item {
    id: view

    property variant contact
    property bool newMessage: false

    Component {
        id: newHeaderComponent
        NewMessageHeader {
            id: newHeader
            width: view.width
            height: 100
        }
    }

    Component {
        id: headerComponent
        MessagesHeader {
            id: header
            contact: view.contact
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
        onNewMessage: {
            if (messagesLoader.sourceComponent) {
                messagesLoader.item.addMessage(message)
            }
        }
        anchors.bottom: parent.bottom
    }
}

