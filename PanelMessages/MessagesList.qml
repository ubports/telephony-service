import QtQuick 1.1

Item {
    id: messageList

    property alias model: listView.model
    property string filter

    ListView {
        id: listView
        anchors.fill: parent
        clip: true
        delegate: Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: message.visible ? 45 : 0
            MessageDelegate {
                id: message
                anchors.fill: parent
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                onClicked: telephony.startChat(contact)
                visible: contact.displayName.toLowerCase().indexOf(filter.toLowerCase()) >= 0 ? true : false
            }
        }
    }
}
