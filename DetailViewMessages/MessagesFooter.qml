import QtQuick 1.1
import "../Widgets"
import Ubuntu.Components 0.1

FocusScope {
    id: footer
   
    property bool validRecipient: false

    signal newMessage(string message)

    height: 36

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
        height: 1
        color: "white"
    }

    Item {
        anchors.top: separator.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        // FIXME: do not use TextField for a simple input field
        TextField {
            id: entry

            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: sendButton.left
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            focus: true
            // send message if return was pressed
            Keys.onEnterPressed: sendButton.clicked()
            Keys.onEscapePressed: text = ""
        }

        Button {
            id: sendButton

            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.top: entry.top
            anchors.bottom: entry.bottom
            width: 62
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
