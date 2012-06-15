import QtQuick 1.1
import QtMobility.contacts 1.1
import "../Widgets"

Column {
    id: contactDetailsSection
    visible: details.count + newItems.count > 0

    property alias model: details.model
    property alias delegate: details.delegate

    property variant detailTypeInfo
    property bool editable
    onEditableChanged: if (!editable) newItems.model.clear()

    function save() {
        var added = [];
        for (var i = 0; i < children.length; i++) {
            var child = children[i];
            if (child && child.item && child.item.save && child.item.save instanceof Function &&
               !child.item.deleted) {
                child.item.save();
                if (child.item.added) {
                    added.push(child.item.detail);
                }
            }
        }
        return added;
    }

    function appendNewItem() {
        newItems.model.append({})
    }

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        height: section.paintedHeight + 10
        TextCustom {
            id: section
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text: detailTypeInfo.name
            fontSize: "small"
        }
    }

    Repeater {
        id: details
        // model and delegate are set via property alias
    }

    Repeater {
        id: newItems
        model: ListModel {}
        delegate: Loader {
            id: newItem
            source: detailTypeInfo.delegateSource
            anchors.left: (parent) ? parent.left : undefined
            anchors.right: (parent) ? parent.right : undefined

            Binding { target: item; property: "detailTypeInfo"; value: contactDetailsSection.detailTypeInfo }
            Binding { target: item; property: "editable"; value: contactDetailsSection.editable }
            Binding { target: item; property: "added"; value: true }
            Binding { target: item; property: "detail";
                      value: Qt.createQmlObject("import QtMobility.contacts 1.1; " + detailTypeInfo.newItemType + "{}", newItem) }

            opacity: editable ? 1.0 : 0.0
        }
    }

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        height: Math.max(addText.paintedHeight, addButton.height) + 16
        opacity: editable ? 1.0 : 0.0

        ButtonWithForeground {
            id: addButton
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: 8
            width: 20
            iconSource: "../assets/icon_plus.png"
        }

        TextCustom {
            id: addText
            anchors.left: addButton.right
            anchors.leftMargin: 8
            anchors.verticalCenter: addButton.verticalCenter
            color: "green"
            text: (detailTypeInfo.newItemText) ? detailTypeInfo.newItemText : ""
            fontSize: "x-large"
        }

        MouseArea {
            anchors.fill: parent
            onClicked: appendNewItem()
        }
    }
}
