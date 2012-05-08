import QtQuick 1.1

Column {
    id: view

    property variant contact

    MessagesHeader {
        id: header
        contact: view.contact
        width: view.width
        height: 100
    }

    Messages {
        id: messages
        width: view.width
        height: view.height - footer.height - header.height
    }

    MessagesFooter {
        id: footer
        width: view.width
        height: 100
        onNewMessage: messages.addMessage(message)
    }
}

