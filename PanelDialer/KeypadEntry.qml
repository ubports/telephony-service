import QtQuick 2.0
import Ubuntu.Components 0.1

FocusScope {
    id: keypadEntry

    property alias value: input.text

    height: units.gu(6)

    BorderImage {
        id: background

        anchors.fill: parent
        source: "../assets/keypad_entry_background.png"
        border {left: units.dp(15); right: units.dp(15); top: units.dp(18); bottom: units.dp(1)}
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
    }

    TextInput {
        id: input

        anchors.left: parent.left
        anchors.leftMargin: units.gu(2)
        anchors.right: backspace.left
        anchors.rightMargin: units.gu(1)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: units.dp(1)
        height: font.pixelSize
        horizontalAlignment: TextInput.AlignRight
        text: ""
        font.pixelSize: FontUtils.sizeToPixels("large")
        color: "#e3e5e8"
        focus: true
    }

    BackspaceButton {
        id: backspace
        anchors.right: parent.right
        anchors.rightMargin: units.gu(2)
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
