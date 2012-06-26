import QtQuick 1.1
import "../Widgets"
import "../fontUtils.js" as Font

Item {

    signal newMessage(string message)

    Rectangle {
        id: newMessageBox

        border.color: "black"
        border.width: 1
        radius: 10
        color: "lightGray"
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            right: parent.right
            leftMargin: 20
            rightMargin: 20
        }
        height: 40
        smooth: true

        TextInput {
            id: textInput
            anchors.fill: parent
            anchors.margins: 8
            font.pixelSize: Font.sizeToPixels("x-large")
            cursorVisible: focus
            focus: true
            Keys.forwardTo: [sendButton]
        }

        Button {
            id: sendButton
            anchors {
                right: newMessageBox.right
                rightMargin: 10
                verticalCenter: parent.verticalCenter
            }
            height: 25
            color: "#37b301"
            width: 62
            text: "Send"
            onClicked: {
                footer.newMessage(textInput.text)
                textInput.text = ""
            }
        }
    }
}
