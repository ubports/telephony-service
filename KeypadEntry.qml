import QtQuick 1.1
import "fontUtils.js" as Font

Item {
    id: keypadEntry

    property alias value: input.text

    height: 42

    Rectangle {
        anchors.fill: parent
        color: "#757575"
        border.color: "#b1b1b1"
        border.width: 1
    }

    Row {
        anchors.left: parent.left
        anchors.leftMargin: 9
        anchors.right: parent.right
        anchors.rightMargin: 9
        anchors.verticalCenter: parent.verticalCenter
        layoutDirection: Qt.RightToLeft
        spacing: 4

        AbstractButton {
            id: clearButton

            anchors.top: input.top
            anchors.bottom: input.bottom
            width: childrenRect.width

            onClicked: input.text = ""

            Image {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                smooth: true
                fillMode: Image.PreserveAspectFit
                source: "assets/icon_backspace.png"
            }
        }

        TextInput {
            id: input

            width: parent.width - clearButton.width
            text: "+44 7979"
            font.pixelSize: Font.sizeToPixels("x-large")
            color: "white"
        }
    }
}
