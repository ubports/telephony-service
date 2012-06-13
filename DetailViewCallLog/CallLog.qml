import QtQuick 1.1
import TelephonyApp 0.1
import "../Widgets"

Item {
    property string viewName: "callLog"

    Item {
        id: buttonGroup
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5
        height: childrenRect.height

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            height: 30
            spacing: 1

            // FIXME: use a proper component here once we get the visual designs
            Button {
                id: allButton
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                text: "All"
                width: 70

                onClicked: {
                    proxyModel.onlyMissedCalls = false;
                }
            }

            Button {
                id: missedButton
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                text: "Missed"
                width: 70

                onClicked: {
                    proxyModel.onlyMissedCalls = true;
                }
            }
        }
    }

    CallLogProxyModel {
        id: proxyModel
        logModel: callLogModel
        onlyMissedCalls: false
    }

    ListView {
        id: callLogView
        anchors.top: buttonGroup.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 5
        // FIXME: references to runtime and fake model need to be removed before final release
        model: runtime ? fakeCallLog : proxyModel

        delegate: CallLogDelegate {
            width: parent.width
            onClicked: telephony.showContactDetailsFromId(contactId)
            onActionClicked: telephony.callNumber(phoneNumber)
        }
    }
}
