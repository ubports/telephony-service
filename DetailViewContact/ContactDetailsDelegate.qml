import QtQuick 1.1
import "../Widgets"

Item {
    id: contactDetailsItem
    height: childrenRect.height

    property string actionIcon
    property string value
    property string type
    property bool multiLine: false
    property bool editable: false

    signal clicked(string value)
    signal actionClicked(string value)
    signal fieldValueChanged(string newValue)

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
        height: valueTextMulti.height > 30 ? valueTextMulti.height : 30

        onClicked: contactDetailsItem.clicked(contactDetailsItem.value);

        Text {
            anchors.left: parent.left
            anchors.right: typeText.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 5
            text: contactDetailsItem.value
            visible: !contactDetailsItem.editable
        }

        TextInput {
            id: valueText
            anchors.left: parent.left
            anchors.right: typeText.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 5
            text: value
            visible: contactDetailsItem.editable && !contactDetailsItem.multiLine
            onTextChanged: contactDetailsItem.fieldValueChanged(text)
        }

        TextEdit {
            id: valueTextMulti
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 5
            height: paintedHeight
            text: contactDetailsItem.value
            visible: contactDetailsItem.editable && contactDetailsItem.multiLine
        }

        Text {
            id: typeText
            anchors.right: parent.right
            anchors.top: valueTextMulti.top
            anchors.rightMargin: 5
            text: contactDetailsItem.type
        }
    }

    Rectangle {
        id: actionBox
        border.color: "black"
        color: "white"
        width: 30
        height: parent.height
        anchors.top: parent.top
        anchors.bottom: contentBox.bottom
        anchors.right: parent.right

        IconButton {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 5
            height: width

            icon: contactDetailsItem.actionIcon

            onClicked: contactDetailsItem.actionClicked(contactDetailsItem.value);
        }
    }
}
