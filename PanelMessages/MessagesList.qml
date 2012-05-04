import QtQuick 1.1

Item {
    id: messageList

    property alias model: listView.model

    ListView {
        id: listView
        anchors.fill: parent
        clip: true
        delegate: Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: 45
            MessageDelegate {
                anchors.fill: parent
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                onClicked: telephony.startChat(contact)
            }
        }
    }
}
