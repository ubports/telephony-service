import QtQuick 2.0
import Ubuntu.Components 0.1

FocusScope {
    id: keypadEntry

    property alias value: input.text
    property alias input: input
    property alias placeHolder: hint.text
    property alias placeHolderPixelFontSize: hint.font.pixelSize

    height: units.gu(7.5)

    Label {
        id: dots
        clip: true
        width: input.contentWidth > input.width ? contentWidth : 0
        visible: input.contentWidth > input.width
        anchors.left: parent.left
        text: "..."
        font.pixelSize: units.dp(43)
        font.weight: Font.Light
        font.family: "Ubuntu"
        color: "#464646"
    }

    TextInput {
        id: input

        anchors.left: dots.right
        anchors.right: parent.right
        anchors.rightMargin: units.gu(2)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: units.dp(-1)
        horizontalAlignment: TextInput.AlignRight
        text: ""
        font.pixelSize: units.dp(43)
        font.weight: Font.Light
        font.family: "Ubuntu"
        color: "#464646"
        focus: true
        cursorVisible: true
        clip: true
        opacity: 0.9

        // force cursor to be always visible
        onCursorVisibleChanged: {
            if (!cursorVisible)
                cursorVisible = true
        }
    }

    MouseArea {
        anchors.fill: input
        property bool held: false
        onClicked: {
            input.cursorPosition = input.positionAt(mouseX,TextInput.CursorOnCharacter)
        }
        onPressAndHold: {
            if (input.text != "") {
                held = true
                input.selectAll()
                input.copy()
            } else {
                input.paste()
            }
        }
        onReleased: {
            if(held) {
                input.deselect()
                held = false
            }

        }
    }

    Label {
        id: hint
        visible: input.text == ""
        anchors.centerIn: input
        text: ""
        font.pixelSize: units.dp(20)
        font.weight: Font.Light
        font.family: "Ubuntu"
        color: "#464646"
        opacity: 0.9
    }
}
