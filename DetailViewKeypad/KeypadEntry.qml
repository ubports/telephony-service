import QtQuick 1.1
import "../Widgets"
import "../fontUtils.js" as Font

Item {
    id: keypadEntry

    property alias value: input.text

    height: 49

    BorderImage {
        id: background

        anchors.fill: parent
        source: "../assets/keypad_entry_background.png"
        border {left: 15; right: 15; top: 18; bottom: 1}
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
    }

    TextInput {
        id: input

        anchors.left: parent.left
        anchors.leftMargin: 19
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 1
        height: font.pixelSize
        horizontalAlignment: TextInput.AlignRight
        text: "+44 01234 56789"
        font.pixelSize: Font.sizeToPixels("x-large")
        color: "#e3e5e8"
    }
}
