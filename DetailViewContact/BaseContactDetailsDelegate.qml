import QtQuick 1.1
import "../Widgets"

Item {
    id: contactDetailsItem
    height: editable ? editableGroup.height : readOnlyGroup.height

    property variant detail
    property variant detailTypeInfo

    property bool editable: false
    property bool added: false

    signal clicked(string value)
    signal actionClicked(string value)
    signal deleteClicked()

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

    Item {
        id: readOnlyGroup
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: childrenRect.height
        visible: !editable

        AbstractButton {
            id: contentBox

            anchors.left: parent.left
            anchors.right: actionBox.left
            anchors.top: parent.top
            height: Math.max(childrenRect.height, 36)

            onClicked: contactDetailsItem.clicked(contactDetailsItem.value);

            Item {
                id: readOnlyContentBox

                anchors.left: parent.left
                anchors.right: typeText.left
                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                height: childrenRect.height + 8
            }

            TextCustom {
                id: typeText
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: 8
                anchors.topMargin: 8
                text: {
                    // The backend supports multiple types but we can just handle one,
                    // so let's pick just the first
                    var parts = detail.contexts.toString().split(",")
                    for (var i = 0; i < parts.length; i++) {
                        if (parts[i].indexOf("type=") == 0) return parts[i].substring(5)
                    }
                    return "";
                }
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

            Button {
                anchors.fill: parent

                iconSource: detailTypeInfo.actionIcon

                onClicked: contactDetailsItem.actionClicked(contactDetailsItem.value);
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
            height: Math.max(editableContentBox.height, typeEditor.paintedHeight)

            ButtonWithForeground {
                id: removeButton

                anchors.left: parent.left
                anchors.leftMargin: -10
                anchors.verticalCenter: parent.verticalCenter
                width: 20
                iconSource: "../assets/icon_minus.png"

                onClicked: deleteClicked()
            }

            Item {
                id: editableContentBox

                anchors.left: parent.left
                anchors.right: typeEditor.left
                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.leftMargin: 16
                anchors.rightMargin: 8
                height: childrenRect.height + 16

                visible: editable
            }

            TextCustom {
                id: typeEditor
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: 8
                anchors.topMargin: 8
                text: typeText.text
                fontSize: "large"
                color: "lightgrey"
            }
        }
    }
}
