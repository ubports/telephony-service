import QtQuick 2.0
import "DetailTypeUtilities.js" as DetailTypes
import Ubuntu.Components 0.1

Item {
    id: chooser

    property variant contact
    signal selected(variant detailType)

    property variant detail
    property bool opened: false
    property int menuHeight: (opened) ? options.childrenRect.height : 0

    function open() {
        if (opened) return
        optionsList.model = DetailTypes.getTypesWithNoItems(contact);
        chooser.opened = true
    }

    function close() {
        if (!opened) return
        optionsList.model = null
        chooser.opened = false
    }

    onDetailChanged: close()

    Item {
        id: current
        anchors.fill: parent

        Button {
            id: addButton
            anchors.left: parent.left
            anchors.leftMargin: units.gu(1)
            anchors.verticalCenter: parent.verticalCenter
            width: units.gu(2)
            iconSource: "../assets/edit_contact_mode_add.png"
            ItemStyle.class: "transparent-button"
        }

        Label {
            id: addText

            anchors.left: addButton.right
            anchors.leftMargin: units.gu(0.5)
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: units.dp(-1)
            fontSize: "x-small"
            font.italic: true
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
            text: "Add another field"
        }

        MouseArea {
            anchors.fill: parent
            onClicked: (opened) ? close() : open()
        }
    }

    Item {
        id: optionsPositioner

        anchors.left: parent.left
        anchors.leftMargin: units.dp(-1)
        anchors.right: parent.right
        anchors.top: current.bottom
    }

    Column {
        id: options

        // FIXME: We need to reparent this way so that this will catch all the clicks
        // instead of other components declared later in the hierarchy
        parent: chooser.opened ? scrollArea.contentItem : null
        z: 9999

        // FIXME: shaky positioning; breaks if the parent moves or if
        // optionsPositioner moves.
        onParentChanged: updatePosition()
        function updatePosition() {
            var position = optionsPositioner.mapToItem(parent, 0, 0)
            x = position.x
            y = position.y
        }

        width: optionsPositioner.width
        opacity: (optionsList.model && optionsList.model.length > 1) ? 1.0 : 0.0

        Repeater {
            id: optionsList

            delegate: AbstractButton {
                anchors.left: (parent) ? parent.left : undefined
                anchors.right: (parent) ? parent.right : undefined
                height: optionText.paintedHeight + units.gu(3)

                onClicked: {
                    selected(modelData)
                    chooser.close()
                }

                BorderImage {
                    id: background

                    source: "../assets/combobox_item.png"
                    anchors.fill: parent
                    border {top: 0; right: units.dp(2); bottom: units.dp(2); left: units.dp(3)}
                }

                Label {
                    id: optionText

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: units.gu(2)
                    fontSize: "small"
                    elide: Text.ElideRight
                    color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
                    style: Text.Raised
                    styleColor: "white"
                    text: modelData.name
                }
            }
        }

        BorderImage {
            id: shadow

            anchors.left: parent.left
            anchors.right: parent.right
            source: "../assets/combobox_bottom_shadow.png"
            border {top: 0; right: 0; bottom: units.dp(2); left: units.dp(2)}
        }
    }
}
