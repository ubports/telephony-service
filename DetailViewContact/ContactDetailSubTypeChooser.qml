import QtQuick 1.1
import "../Widgets"
import "DetailTypeUtilities.js" as DetailUtils

Item {
    id: chooser

    property variant detail
    property variant detailTypeInfo
    property alias selectedValue: currentText.text

    onDetailChanged: optionsList.model = null
    height: current.height
    width: Math.max(current.width, options.width)

    Button {
        id: current
        anchors.left: parent.left
        anchors.top: parent.top

        onClicked: {
            optionsList.model = (detailTypeInfo.subTypes) ? detailTypeInfo.subTypes : []
        }

        TextCustom {
            id: currentText
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right:  parent.right
            anchors.margins: 5
            fontSize: "x-large"
            text: {
                // Use first allowed subtype as default in case there's no detail or no subType
                var subType = DetailUtils.getDetailSubType(detail)
                if (subType == "" && detailTypeInfo.subTypes) return detailTypeInfo.subTypes[0]
                else return subType
            }
        }
    }

    Column {
        id: options

        // FIXME: We need to reparent this way so that this will catch all the clicks
        // instead of other components declared later in the hierarchy
        parent: contactDetails
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        z: 9999

        opacity: (optionsList.model && optionsList.model.length > 1) ? 1.0 : 0.0
        height: childrenRect.height

        Repeater {
            id: optionsList
            delegate: Button {
                anchors.left: (parent) ? parent.left : undefined
                anchors.right: (parent) ? parent.right : undefined
                height: (opacity == 0.0) ? 0 : optionText.paintedHeight + 20
                opacity: (optionText == currentText.text) ? 0.0 : 1.0

                onClicked: {
                    currentText.text = modelData
                    optionsList.model = null
                }

                TextCustom {
                    id: optionText
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.right:  parent.right
                    anchors.margins: 15
                    fontSize: "x-large"
                    text: modelData
                }
            }
        }
    }
}
