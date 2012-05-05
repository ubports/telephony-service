import QtQuick 1.1

//FIXME: remove top level item
Item {
    id: view

    property variant contact

    width: 575
    height: 800

    Column {
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
}
