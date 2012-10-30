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
        id: groupComponent

        GroupDelegate {
            anchors.left: parent ? parent.left : undefined
            anchors.right: parent ? parent.right : undefined
            height: 64
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

            property string contactId: model ? model.contactId : ""
            property string contactAlias: model ? model.contactAlias : ""
            property url contactAvatar: model ? model.contactAvatar : ""
            property variant timestamp: model ? model.timestamp : null
            property bool incoming: model ? model.incoming : false
            property string itemType: model ? model.itemType : "none"
            property QtObject item: model ? model.item : null
            property variant events: model ? model.events : null

            anchors.left: parent.left
            anchors.right: parent.right

            sourceComponent: {
                switch (itemType) {
                case "message":
                    messageComponent;
                case "call":
                    callLogComponent;
                case "group":
                    groupComponent;
                }
            }
        }
    }

    LocalWidgets.ScrollbarForListView {
        view: listView
    }
}
