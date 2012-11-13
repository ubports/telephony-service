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
        }
    }

    ListView {
        id: listView
        anchors.fill: parent
        clip: true
        delegate: ListItem.Base {
            id: delegate
            anchors.left: parent.left
            anchors.right: parent.right
            showDivider: true
            __height: units.gu(7)

            onClicked: {
                telephony.showCommunication(groupingProperty, item[groupingProperty], model.contactId, true);
            }

            selected: communicationsDelegate.selected

            Loader {
                id: communicationsDelegate

                signal clicked(variant mouse)
                onClicked: delegate.clicked(mouse)

                anchors.fill: parent
                property string contactId: (model && model.contactId) ? model.contactId : ""
                property string contactAlias: (model && model.contactAlias) ? model.contactAlias : ""
                property url contactAvatar: (model && model.contactAvatar) ? model.contactAvatar : ""
                property variant timestamp: (model && model.timestamp) ? model.timestamp : null
                property bool incoming: (model && model.incoming) ? model.incoming : false
                property string itemType: (model && model.itemType) ? model.itemType : "none"
                property QtObject item: (model && model.item) ? model.item : null
                property variant events: (model && model.events) ? model.events : null
                property bool selected: isSelected()

                function isSelected() {
                    if (!model || !model.groupingProperty) {
                        return false;
                    }

                    if (!telephony.view || !telephony.view.filterProperty) {
                        return false;
                    }

                    return (telephony.view.filterProperty == model.groupingProperty) &&
                            (telephony.view.filterValue == model.item[model.groupingProperty]);
                }

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
