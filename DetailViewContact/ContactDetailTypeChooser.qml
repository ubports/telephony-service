import QtQuick 1.1
import "../Widgets"
import "DetailTypeUtilities.js" as DetailTypes

Item {
    id: chooser

    property variant contact
    signal selected(variant detailType)

    onContactChanged: optionsList.model = null
    height: options.height + newButton.height

    ColoredButton {
        id: newButton
        color: "transparent"
        borderColor: "black"
        borderWidth: 1
        radius: 0
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top


        height: createText.paintedHeight + createText.anchors.margins * 2

        onClicked: {
            console.log("Selected:  " + contact);
            optionsList.model = DetailTypes.getTypesWithNoItems(contact)
            console.log("Selected:  " + optionsList.model);
        }

        TextCustom {
            id: createText
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right:  parent.right
            anchors.margins: 5
            fontSize: "x-large"
            text: "Add another field"
        }
    }

    Column {
        id: options
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 15
        anchors.top: newButton.bottom
        anchors.topMargin: -10
        opacity: (optionsList.model && optionsList.model.length > 0) ? 1.0 : 0.0
        height: childrenRect.height

        Repeater {
            id: optionsList
            delegate: ColoredButton {
                anchors.left: (parent) ? parent.left : undefined
                anchors.right: (parent) ? parent.right : undefined
                color: "white"
                borderColor: "black"
                borderWidth: 1
                height: optionText.paintedHeight + 20

                onClicked: {
                    selected(modelData)
                    optionsList.model = null
                }

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
