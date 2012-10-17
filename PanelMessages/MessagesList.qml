import QtQuick 1.1
import "../Widgets"

Item {
    id: messageList

    property alias model: listView.model

    ListView {
        id: listView
        anchors.fill: parent
        clip: true
        delegate: MessageDelegate {
            id: messageDelegate

            anchors.left: parent.left
            anchors.right: parent.right
            onClicked: {
                var contact = contactModel.contactFromPhoneNumber(phoneNumber)
                var id
                if (contact) {
                    id = contact.id
                }
                telephony.startChat(id, phoneNumber, true)
            }
            selected: telephony.messages.loaded
                      && !telephony.view.newMessage
                      && contactModel.comparePhoneNumbers(telephony.view.number, phoneNumber)
        }
    }

    ScrollbarForListView {
        view: listView
    }
}
