import QtQuick 1.1
import "../Widgets"

Item {
    id: contactDetailsItem
    height: childrenRect.height

    property bool editable: false
    signal clicked(string section, string value)
    signal actionClicked(string section, string value)

    function save() {
        var newValue

        if (section == "Address") {
            newValue = valueTextMulti.text
        } else {
            newValue = valueText.text
        }

        contactdetails.set(index, { "value": newValue })
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

        onClicked: contactDetailsItem.clicked(section, value);

        TextInput {
            id: valueText
            anchors.left: parent.left
            anchors.right: typeText.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 5
            text: value
            visible: contactDetailsItem.editable && (section != "Address")
        }

        Text {
            anchors.left: parent.left
            anchors.right: typeText.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 5
            text: value
            visible: !contactDetailsItem.editable && (section != "Address")
        }

        // Used to edit the address
        TextEdit {
            id: valueTextMulti
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 5
            height: paintedHeight
            text: value
            visible: contactDetailsItem.editable && (section == "Address")
        }

        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 5
            height: paintedHeight
            text: value
            visible: !contactDetailsItem.editable && (section == "Address")
        }

        Text {
            id: typeText
            anchors.right: parent.right
            anchors.top: valueTextMulti.top
            anchors.rightMargin: 5
            text: type
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

            icon: actionIcon

            onClicked: contactDetailsItem.actionClicked(section, value);
        }
    }

    Row {
        id: addItemRow

        property bool shouldShow: contactDetailsItem.editable && contactDetailsItem.ListView.nextSection != section
        height: shouldShow ? 20 : 0
        anchors.left: removeButton.left
        anchors.top: contentBox.bottom
        anchors.right: actionBox.right

        opacity: shouldShow ? 1 : 0

        Behavior on height { PropertyAnimation { duration: 125 } }
        Behavior on opacity { PropertyAnimation { duration: 125 } }

        IconButton {
            id: addButton
            icon: "../assets/icon_plus.png"
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: height

            onClicked: {
                contactdetails.insert(index+1, { "section": section, "type":"Work", "value":"", "actionIcon":actionIcon });
            }

        }

        TextCustom {
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 5
            text: "Add another entry"
            visible: addButton.visible
            height: addItemRow.shouldShow ? paintedHeight : 0
        }
    }
}
