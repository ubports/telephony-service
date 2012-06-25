import QtQuick 1.1
import "../Widgets"
import "DetailTypeUtilities.js" as DetailUtils

Item {
    id: contactDetailsItem

    /* For deleted items it's not enough to hide them, they will still take space in
       the layout. We also need to set the height to zero to make them completely go away */
    height: (deleted) ? 0 : (((editable) ? editableGroup.height : readOnlyGroup.height) + bottomSeparatorLine.height)
    opacity: (deleted) ? 0.0 : 1.0

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

        if (saveDetail instanceof Function) saveDetail();
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
        height: visible ? 2 : 0
        source: "../Widgets/artwork/ListItemSeparator.png"
        visible: contactDetailsItem.bottomSeparator
    }

    Item {
        id: readOnlyGroup

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: contentBox.height + 2*contentBox.anchors.topMargin
        visible: !editable

        AbstractButton {
            id: contentBox

            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.right: separator.left
            anchors.rightMargin: 7
            anchors.top: parent.top
            anchors.topMargin: 9
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
            id: separator

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: actionBox.left
            width: 1
            color: "black"
            opacity: 0.1
        }

        AbstractButton {
            id: actionBox

            width: 40
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            onClicked: contactDetailsItem.actionClicked(contactDetailsItem.value);

            Image {
                anchors.centerIn: parent
                width: 16
                sourceSize.width: width
                fillMode: Image.PreserveAspectFit

                source: (detailTypeInfo.actionIcon) ? detailTypeInfo.actionIcon : "../assets/icon_chevron_right.png"
            }
        }
    }

    Item {
        id: editableGroup

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        visible: editable
        height: childrenRect.height

        Rectangle {
            id: editorArea
            border.color: "black"
            border.width: 1
            color: "white"

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 10
            height: Math.max(editableContentBox.height, subTypeEditor.height)

            ButtonWithForeground {
                id: removeButton

                anchors.left: parent.left
                anchors.leftMargin: -10
                anchors.verticalCenter: parent.verticalCenter
                width: 20
                iconSource: "../assets/icon_minus.png"

                onClicked: {
                    deleted = true;
                    deleteClicked();
                }
            }

            Item {
                id: editableContentBox

                anchors.left: parent.left
                anchors.right: subTypeEditor.left
                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.leftMargin: 16
                anchors.rightMargin: 8
                height: childrenRect.height + 16

                visible: editable
            }

            ContactDetailSubTypeChooser {
                id: subTypeEditor
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: 8
                anchors.topMargin: 8
                detailTypeInfo: contactDetailsItem.detailTypeInfo
                detail: contactDetailsItem.detail

                opacity: editable ? 1.0 : 0.0
            }
        }
    }
}
