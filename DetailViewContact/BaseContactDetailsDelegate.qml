import QtQuick 1.1
import "../Widgets"

Item {
    id: contactDetailsItem
    height: childrenRect.height

    property string actionIcon
    property string type
    property bool editable: false

    property variant contactModelItem

    signal clicked(string value)
    signal actionClicked(string value)
    signal fieldValueChanged(string newValue)

    /* Internal properties, use by derived components */
    property variant readOnlyContentBox: readOnlyContentBox
    property variant editableContentBox: editableContentBox

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

    IconButton {
        id: removeButton

        anchors.left: parent.left
        anchors.verticalCenter: contentBox.verticalCenter
        width: contactDetailsItem.editable ? 20 : 0
        height: width
        icon: "../assets/icon_minus.png"
        opacity: contactDetailsItem.editable ? 1 : 0

        onClicked: {
            contactdetails.remove(index)
        }

        Behavior on width { PropertyAnimation { duration: 125 } }
        Behavior on opacity { PropertyAnimation { duration: 125 } }
    }

    ColoredButton {
        id: contentBox
        borderColor: "black"
        borderWidth: 1
        color: "white"
        radius: 0

        anchors.left: removeButton.right
        anchors.right: actionBox.left
        anchors.top: parent.top
        height: Math.max(childrenRect.height, 36)

        onClicked: if (!editable) contactDetailsItem.clicked(contactDetailsItem.value);

        Item {
            id: readOnlyContentBox

            anchors.left: parent.left
            anchors.right: typeText.left
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            height: childrenRect.height + 8

            opacity: editable ? 0.0 : 1.0
        }

        Item {
            id: editableContentBox

            anchors.left: parent.left
            anchors.right: typeText.left
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            height: childrenRect.height + 8

            opacity: editable ? 1.0 : 0.0
        }

        TextCustom {
            id: typeText
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: 8
            anchors.topMargin: 8
            text: contactDetailsItem.type
            fontSize: "large"
            color: "lightgrey"
        }
    }

    Rectangle {
        id: actionBox
        border.color: "black"
        color: "white"
        width: 60
        height: parent.height
        anchors.top: parent.top
        anchors.bottom: contentBox.bottom
        anchors.right: parent.right

        IconButton {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 5
            height: 32

            icon: contactDetailsItem.actionIcon

            onClicked: contactDetailsItem.actionClicked(contactDetailsItem.value);
        }
    }
}
