import QtQuick 1.1
import "../Widgets"
import Ubuntu.Components 0.1
import "DetailTypeUtilities.js" as DetailUtils

AbstractButton {
    id: chooser

    property variant detail
    property variant detailTypeInfo
    property alias selectedValue: currentText.text
    property bool opened: false

    function open() {
        if (opened) return
        optionsList.model = (detailTypeInfo.subTypes) ? detailTypeInfo.subTypes : []
        chooser.opened = true
    }

    function close() {
        if (!opened) return
        optionsList.model = null
        chooser.opened = false
    }

    onDetailChanged: close()
    onClicked: {
        if (opened) close()
        else open()
    }

    Item {
        id: current

        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10

        TextCustom {
            id: currentText

            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: expander.left
            anchors.rightMargin: 10
            elide: Text.ElideRight
            fontSize: "small"
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            text: {
                // Use first allowed subtype as default in case there's no detail or no subType
                var subType = DetailUtils.getDetailSubType(detail)
                if (subType == "" && detailTypeInfo.subTypes) return detailTypeInfo.subTypes[0]
                else return subType
            }
        }

        Image {
            id: expander

            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            width: 8
            source: chooser.opened ? "../assets/edit_contact_dropup_arrow.png" : "../assets/edit_contact_dropdown_arrow.png"
        }
    }

    Item {
        id: optionsPositioner

        anchors.left: parent.left
        anchors.leftMargin: -1
        anchors.right: parent.right
        anchors.top: current.bottom
    }

    Column {
        id: options

        // FIXME: We need to reparent this way so that this will catch all the clicks
        // instead of other components declared later in the hierarchy
        parent: chooser.opened ? contactDetails : null
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
                height: (opacity == 0.0) ? 0 : optionText.paintedHeight + 20
                opacity: (optionText == currentText.text) ? 0.0 : 1.0

                onClicked: {
                    currentText.text = modelData
                    chooser.close()
                }

                BorderImage {
                    id: background

                    source: "../assets/combobox_item.png"
                    anchors.fill: parent
                    border {top: 0; right: 2; bottom: 2; left: 3}
                }

                TextCustom {
                    id: optionText

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 15
                    fontSize: "small"
                    elide: Text.ElideRight
                    color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
                    style: Text.Raised
                    styleColor: "white"
                    text: modelData
                }
            }
        }

        BorderImage {
            id: shadow

            anchors.left: parent.left
            anchors.right: parent.right
            source: "../assets/combobox_bottom_shadow.png"
            border {top: 0; right: 0; bottom: 2; left: 2}
        }
    }
}
