import QtQuick 2.0
import Ubuntu.Components 0.1

FocusScope {
    id: footer
   
    property bool validRecipient: false

    signal newMessage(string message)

    height: units.gu(5)

    Rectangle {
        anchors.fill: parent
        color: "white"
        opacity: 0.3
    }

    Rectangle {
        id: separator

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: units.dp(1)
        color: "white"
    }

    Item {
        anchors.top: separator.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        TextField {
            id: entry

            objectName: "newMessageText"
            anchors.left: parent.left
            anchors.leftMargin: units.gu(1)
            anchors.right: sendButton.left
            anchors.rightMargin: units.gu(1)
            anchors.verticalCenter: parent.verticalCenter
            focus: true
            // send message if return was pressed
            Keys.onReturnPressed: sendButton.clicked()
            Keys.onEscapePressed: text = ""
        }

        Button {
            id: sendButton

            anchors.right: parent.right
            anchors.rightMargin: units.gu(1)
            anchors.top: entry.top
            anchors.bottom: entry.bottom
            width: units.gu(9)
            enabled: validRecipient
            color: "#37b301"
            text: "Send"
            onClicked: {
                if(!validRecipient) {
                    return;
                }
                footer.newMessage(entry.text)
                entry.text = ""
            }
        }
    }
}
