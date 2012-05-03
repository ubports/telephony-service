import QtQuick 1.1

Item {
    id: view
    width: 575
    height: 800   

    Column {
        MessagesHeader {
            id: header
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
