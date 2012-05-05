import QtQuick 1.1

//FIXME: remove the whole file
Item {
    id: view

    property variant contact

    width: 575
    height: 800

    Column {
        NewMessageHeader {
            id: header            
            width: view.width
            height: 100
        }
        NewMessage {
            id: messages
            width: view.width
            height: view.height - footer.height - header.height

        }
        MessagesFooter {
            id: footer
            width: view.width
            height: 100            
        }
    }
}
