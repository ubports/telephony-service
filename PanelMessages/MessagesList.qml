import QtQuick 1.1
import "../Widgets"

// FIXME: the ListView could be the toplevel item
Item {
    id: messageList

    property alias model: listView.model

    ListView {
        id: listView
        // FIXME: after moving the ListView to the toplevel, remove this anchor
        anchors.fill: parent
        clip: true
        // FIXME: No need for the extra item in the delegate
        delegate: Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: messageDelegate.visible ? messageDelegate.height : 0
            MessageDelegate {
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
    }

    ScrollbarForListView {
        view: listView
    }
}
