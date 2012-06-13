import QtQuick 1.1
import "../Widgets"
import "DetailTypeUtilities.js" as DetailTypes

ColoredButton {
    property variant contact

    id: chooser
    color: "transparent"
    borderColor: "black"
    borderWidth: 1
    radius: 0

    height: createText.paintedHeight + createText.anchors.margins * 2

    onClicked: optionsList.model = DetailTypes.getTypesWithNoItems(contact)
    onContactChanged: optionsList.model = null

    TextCustom {
        id: createText
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.right:  parent.right
        anchors.margins: 5
        fontSize: "x-large"
        text: "Add another field"
    }

    Column {
        id: options
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 15
        anchors.bottom: parent.top
        anchors.bottomMargin: -15
        opacity: (optionsList.model && optionsList.model.length > 0) ? 1.0 : 0.0

        Repeater {
            id: optionsList
            delegate: ColoredButton {
                anchors.left: (parent) ? parent.left : undefined
                anchors.right: (parent) ? parent.right : undefined
                color: "white"
                borderColor: "black"
                borderWidth: 1
                height: optionText.paintedHeight + 20

                TextCustom {
                    id: optionText
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.right:  parent.right
                    anchors.margins: 15
                    fontSize: "x-large"
                    text: modelData.name
                }
            }
        }
    }
}
