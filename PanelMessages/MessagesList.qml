import QtQuick 1.1

// FIXME: the ListView could be the toplevel item
Item {
    id: messageList

    property alias model: listView.model
    property string filter

    ListView {
        id: listView
        // FIXME: after moving the ListView to the toplevel, remove this anchor
        anchors.fill: parent
        clip: true
        // FIXME: No need for the extra item in the delegate
        delegate: Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: message.visible ? message.height : 0
            MessageDelegate {
                id: message

                anchors.left: parent.left
                anchors.right: parent.right
                onClicked: telephony.startChat(contact, contact.phoneNumber.number)
                visible: contact.displayName.toLowerCase().indexOf(filter.toLowerCase()) >= 0 ? true : false
                selected: telephony.messages.loaded
                          && !telephony.view.newMessage
                          && telephony.view.contact == contact
                          && telephony.view.number == contact.phoneNumber.number
            }
        }
    }
}
