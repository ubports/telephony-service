import QtQuick 1.1
import "../fontUtils.js" as Font

Item {
    width: 575
    height: 100   

    signal newMessage(string message)

    Rectangle {
        id: searchBox
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
        }

        Rectangle {
            id: sendButton
            anchors {
                right: searchBox.right
                rightMargin: 10
                verticalCenter: parent.verticalCenter
            }
            height: 30
            radius: 10
            color: "darkGray"
            border.width: 1
            border.color: "black"
            width: 80
            smooth: true
            Text {
                text: "send"
                color: "black"
                font.pixelSize: Font.sizeToPixels("large")
                anchors.centerIn: parent
            }

            MouseArea {
                anchors.fill: parent
                onPressed: {
                    footer.newMessage(textInput.text)
                    textInput.text = ""
                }
            }

        }
    }
}
