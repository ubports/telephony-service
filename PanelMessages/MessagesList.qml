import QtQuick 1.1
import "../Widgets"

Item {
    id: messageList

    property alias model: listView.model

    ListView {
        id: listView
        anchors.fill: parent
        clip: true
        delegate: MessageDelegate {
            id: messageDelegate

            anchors.left: parent.left
            anchors.right: parent.right
            onClicked: telephony.startChat(customId, phoneNumber, threadId)
            selected: telephony.messages.loaded
                      && !telephony.view.newMessage
                      && (threadId != "" && (telephony.view.threadId == threadId)
                      || contactModel.comparePhoneNumbers(telephony.view.number, phoneNumber))
        }
    }

    ScrollbarForListView {
        view: listView
    }
}
