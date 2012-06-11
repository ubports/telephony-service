import QtQuick 1.1
import "../Widgets"

Item {
    property string viewName: "callLog"

    Item {
        height: childrenRect.height
        anchors.left: parent.left
        anchors.right: parent.right

        Row {
            Button
        }
    }

    // FIXME: add the All/Missed buttons
    ListView {
        id: callLogView
        height: count * 64
        anchors.fill: parent
        anchors.margins: 5
        model: callLogModel
        delegate: CallLogDelegate {
            onClicked: telephony.showContactDetailsFromId(contactId)
        }
    }
}
