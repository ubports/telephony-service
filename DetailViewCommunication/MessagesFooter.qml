import QtQuick 2.0
import Ubuntu.Components 0.1

FocusScope {
    id: footer
   
    property bool validRecipient: false
    property alias hasFocus: entry.activeFocus
    property bool newConversation: true

    signal newMessage(string message)

    height: entry.height + units.gu(2)

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
            anchors.bottom: parent.bottom
            anchors.bottomMargin: units.gu(1.5)
            focus: true
            font.pixelSize: FontUtils.sizeToPixels("medium")
            font.family: "Ubuntu"

            // send message if return was pressed
            Keys.onReturnPressed: sendButton.clicked(null)
            Keys.onEscapePressed: text = ""
            height: units.gu(4)
            placeholderText: newConversation ? "Compose" : "Reply via SMS"
        }

        Button {
            id: sendButton

            anchors.right: parent.right
            anchors.rightMargin: units.gu(1)
            anchors.top: entry.top
            anchors.bottom: entry.bottom
            width: units.gu(9)
            height: units.gu(4)
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
