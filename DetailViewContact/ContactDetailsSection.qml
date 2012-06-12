import QtQuick 1.1
import QtMobility.contacts 1.1
import "../Widgets"

Column {
    id: contactDetailsSection
    visible: details.count > 0
    property string name
    property alias model: details.model
    property alias delegate: details.delegate
    property string addText: "Add another <type of detail>"
    property bool editable

    function save() {
        for (var i = 0; i < children.length; i++) {
            var saver = children[i].save;
            if (saver && saver instanceof Function) saver();
        }
    }

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        height: section.paintedHeight + 10
        TextCustom {
            id: section
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text: name
            fontSize: "small"
        }
    }

    Repeater {
        id: details
        // model and delegate are set via property alias
    }

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height + 16
        opacity: editable ? 1.0 : 0.0

        IconButton {
            id: addButton
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: 8
            width: 20
            height: width
            icon: "../assets/icon_plus.png"
        }

        TextCustom {
            anchors.left: addButton.right
            anchors.leftMargin: 8
            anchors.verticalCenter: addButton.verticalCenter
            color: "green"
            text: addText
            fontSize: "x-large"
        }
    }
}
