import QtQuick 2.0
import "../Widgets" as LocalWidgets

Item {
    id: messageList

    property alias model: listView.model

    Component {
        id: messageComponent

        MessageDelegate {
            id: messageDelegate

            anchors.left: parent ? parent.left : undefined
            anchors.right: parent ? parent.right : undefined
            onClicked: {
                var contact = contactModel.contactFromPhoneNumber(phoneNumber)
                var id
                if (contact) {
                    id = contact.id
                }
                telephony.startChat(id, phoneNumber, true)
            }
            /*selected: telephony.messages.loaded
                      && !telephony.view.newMessage
                      && contactModel.comparePhoneNumbers(telephony.view.number, phoneNumber)*/
        }
    }

    Component {
        id: callLogComponent

        CallLogDelegate {
            id: callLogDelegate

            anchors.left: parent ? parent.left : undefined 
            anchors.right: parent ? parent.right : undefined
            onClicked: {
                var contact = contactModel.contactFromPhoneNumber(phoneNumber)
                var id
                if (contact) {
                    id = contact.id
                }
                telephony.call(id, phoneNumber)
            }
            /*selected: telephony.messages.loaded
                      && !telephony.view.newMessage
                      && contactModel.comparePhoneNumbers(telephony.view.number, phoneNumber)*/
        }
    }

    ListView {
        id: listView
        anchors.fill: parent
        clip: true
        delegate: Loader {
            id: communicationsDelegate
            sourceComponent: {
                switch (itemType) {
                case "message":
                    return messageComponent;
                case "call":
                    return callLogComponent;
                }
            }
        }
    }

    LocalWidgets.ScrollbarForListView {
        view: listView
    }
}
