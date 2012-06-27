import QtQuick 1.1
import TelephonyApp 0.1
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

    ListSectionHeader {
        anchors.left: parent.left
        anchors.right: parent.right
        text: detailTypeInfo.name
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
                      value: Qt.createQmlObject("import TelephonyApp 0.1; " + detailTypeInfo.newItemType + "{}", newItem) }

            opacity: editable ? 1.0 : 0.0
        }
    }

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        height: editable ? 32 : 0
        opacity: editable ? 1.0 : 0.0
        Behavior on height {StandardAnimation {}}
        Behavior on opacity {StandardAnimation {}}

        ButtonWithForeground {
            id: addButton
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            width: 12
            iconSource: "../assets/edit_contact_mode_add.png"
        }

        TextCustom {
            id: addText

            anchors.left: addButton.right
            anchors.leftMargin: 5
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -1
            fontSize: "small"
            font.italic: true
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
            text: (detailTypeInfo.newItemText) ? detailTypeInfo.newItemText : ""
        }

        MouseArea {
            anchors.fill: parent
            onClicked: appendNewItem()
        }
    }
}
