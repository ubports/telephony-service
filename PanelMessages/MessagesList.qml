import QtQuick 1.1
import "../Widgets"

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
            height: messageDelegate.visible ? messageDelegate.height : 0
            MessageDelegate {
                id: messageDelegate

                anchors.left: parent.left
                anchors.right: parent.right
                // FIXME: need to match the contact with the number
                onClicked: telephony.startChat(null, phoneNumber)
                selected: telephony.messages.loaded
                          && !telephony.view.newMessage
                          //&& telephony.view.contact == contact
                          && telephony.view.number == phoneNumber
            }
        }
    }

    Scrollbar {
        targetFlickable: listView
    }
}
