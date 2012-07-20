import QtQuick 1.1
import "../Widgets"
import "../fontUtils.js" as Font

FocusScope {
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
        anchors.right: backspace.left
        anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 1
        height: font.pixelSize
        horizontalAlignment: TextInput.AlignRight
        text: ""
        font.pixelSize: Font.sizeToPixels("x-large")
        color: "#e3e5e8"
        focus: true
    }

    BackspaceButton {
        id: backspace
        anchors.right: parent.right
        anchors.rightMargin: 19
        anchors.verticalCenter: input.verticalCenter
        onClicked:  {
            if (input.cursorPosition != 0)  {
                var position = input.cursorPosition;
                input.text = input.text.slice(0, input.cursorPosition - 1) + input.text.slice(input.cursorPosition);
                input.cursorPosition = position - 1;
            }
        }
    }
}
