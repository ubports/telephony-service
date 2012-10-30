import QtQuick 2.0
import "../Widgets" as LocalWidgets
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

Item {
    id: messageList

    property alias model: listView.model

    Component {
        id: messageComponent

        MessageDelegate {
            id: messageDelegate

            /*selected: telephony.messages.loaded
                      && !telephony.view.newMessage
                      && contactModel.comparePhoneNumbers(telephony.view.number, phoneNumber)*/
        }
    }

    Component {
        id: groupComponent

        GroupDelegate {
            id: groupDelegate
        }
    }

    Component {
        id: callLogComponent

        CallLogDelegate {
            id: callLogDelegate

            /*selected: telephony.messages.loaded
                      && !telephony.view.newMessage
                      && contactModel.comparePhoneNumbers(telephony.view.number, phoneNumber)*/
        }
    }

    ListView {
        id: listView
        anchors.fill: parent
        clip: true
        delegate: ListItem.Base {
            anchors.left: parent.left
            anchors.right: parent.right
            showDivider: true
            __height: 58

            Loader {
                id: communicationsDelegate

                anchors.fill: parent
                property string contactId: model ? model.contactId : ""
                property string contactAlias: model ? model.contactAlias : ""
                property url contactAvatar: model ? model.contactAvatar : ""
                property variant timestamp: model ? model.timestamp : null
                property bool incoming: model ? model.incoming : false
                property string itemType: model ? model.itemType : "none"
                property QtObject item: model ? model.item : null
                property variant events: model ? model.events : null

                sourceComponent: {
                    switch (itemType) {
                    case "message":
                        messageComponent;
                        break;
                    case "call":
                        callLogComponent;
                        break;
                    case "group":
                        groupComponent;
                        break;
                    }
                }
            }
        }
    }

    LocalWidgets.ScrollbarForListView {
        view: listView
    }
}
