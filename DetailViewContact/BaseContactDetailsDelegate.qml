import QtQuick 2.0
import "../Widgets" as LocalWidgets
import Ubuntu.Components 0.1
import "DetailTypeUtilities.js" as DetailUtils

FocusScope {
    id: contactDetailsItem

    /* For deleted items it's not enough to hide them, they will still take space in
       the layout. We also need to set the height to zero to make them completely go away.
       There is a 2 pixels vertical spacing between fields in edit mode.
    */
    height: (deleted) ? 0 : (((editable) ? editableGroup.height + units.dp(2) : readOnlyGroup.height) + bottomSeparatorLine.height - units.dp(1))
    opacity: (deleted) ? 0.0 : 1.0

    state: "read"
    states: [
        State {
            name: "read"
        },
        State {
            name: "edit"
            when: contactDetailsItem.editable
        }
    ]
    transitions: Transition {
        LocalWidgets.StandardAnimation { property: "height" }
    }

    property variant detail
    property variant detailTypeInfo

    property bool editable: false
    property bool added: false
    /* We need to keep track of the deleted state of a detail because it will be
       actually deleted from the model only when we save the contact, even if we
       have already called contact.removeDetail() on it. */
    property bool deleted: false

    property bool bottomSeparator: false

    signal clicked(string value)
    signal actionClicked(string value)
    signal deleteClicked()

    /* Internal properties, use by derived components */
    property variant readOnlyContentBox: readOnlyContentBox
    property variant editableContentBox: editableContentBox

    function save() {
        // First save the subType of the detail, then check if we are being
        // subclassed and if the subclass defines its own saving function, and
        // in that case call it

        if (subTypeEditor.selectedValue != "")
            DetailUtils.setDetailSubType(detail, subTypeEditor.selectedValue);

        if (saveDetail instanceof Function) return saveDetail();
        else return true;
    }

    ListView.onRemove: SequentialAnimation {
        PropertyAction { target: contactDetailsItem; property: "ListView.delayRemove"; value: true }
        NumberAnimation { target: contactDetailsItem; property: "opacity"; to: 0; duration: 250 }
        NumberAnimation { target: contactDetailsItem; property: "height"; to: 0; duration: 250 }
        PropertyAction { target: contactDetailsItem; property: "ListView.delayRemove"; value: false }
    }

    ListView.onAdd: SequentialAnimation {
        NumberAnimation { target: contactDetailsItem; property: "opacity"; from: 0; duration: 250 }
        NumberAnimation { target: contactDetailsItem; property: "height"; from: 0; duration: 250 }
    }

    Image {
        id: bottomSeparatorLine

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: visible ? units.dp(2) : 0
        source: "../Widgets/artwork/ListItemSeparator.png"
        visible: contactDetailsItem.bottomSeparator
    }

    Item {
        id: readOnlyGroup

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        // center contentBox vertically
        height: contentBox.height + 2*contentBox.anchors.topMargin
        opacity: editable ? 0.0 : 1.0
        Behavior on opacity {LocalWidgets.StandardAnimation {}}

        AbstractButton {
            id: contentBox

            anchors.left: parent.left
            anchors.leftMargin: units.dp(8)
            anchors.right: readOnlySeparator.left
            anchors.rightMargin: units.dp(7)
            anchors.top: parent.top
            anchors.topMargin: units.dp(9)
            height: childrenRect.height

            onClicked: contactDetailsItem.clicked(contactDetailsItem.value);

            Item {
                id: readOnlyContentBox

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: childrenRect.height
            }
        }

        Rectangle {
            id: readOnlySeparator

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: actionBox.left
            width: units.dp(1)
            color: "black"
            opacity: 0.1
        }

        AbstractButton {
            id: actionBox

            width: units.dp(50)
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            onClicked: contactDetailsItem.actionClicked(contactDetailsItem.value);

            Image {
                anchors.centerIn: parent
                width: units.dp(16)
                sourceSize.width: width
                fillMode: Image.PreserveAspectFit

                source: (detailTypeInfo.actionIcon) ? detailTypeInfo.actionIcon : ""
            }
        }
    }

    EditBox {
        id: editableGroup

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        opacity: editable ? 1.0 : 0.0
        Behavior on opacity {LocalWidgets.StandardAnimation {}}

        Item {
            parent: editableGroup.leftBox
            anchors.left: parent.left
            anchors.right: parent.right
            height: editableContentBox.height

            ButtonWithForeground {
                id: removeButton

                anchors.left: parent.left
                anchors.leftMargin: units.dp(10)
                anchors.verticalCenter: editableContentBox.verticalCenter
                width: units.dp(12)
                iconSource: "../assets/edit_contact_mode_remove.png"

                onClicked: {
                    deleted = true;
                    deleteClicked();
                }
            }

            Item {
                id: editableContentBox

                anchors.left: removeButton.right
                anchors.right: parent.right
                anchors.leftMargin: units.dp(5)
                anchors.rightMargin: units.dp(5)
                height: childrenRect.height
            }
        }

        ContactDetailSubTypeChooser {
            id: subTypeEditor

            parent: editableGroup.rightBox
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            width: units.dp(90)
            detailTypeInfo: contactDetailsItem.detailTypeInfo
            detail: contactDetailsItem.detail
        }
    }
}
