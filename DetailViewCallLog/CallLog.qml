import QtQuick 1.1

Item {
    property string viewName: "callLog"

    // FIXME: add the All/Missed buttons
    ListView {
        id: callLogView
        height: count * 64
        anchors.fill: parent
        anchors.margins: 5
        interactive: false
        model: callLogModel
        delegate: CallLogDelegate {
            //onClicked: telephony.showContactDetails(calllog.get(index))
        }
    }
}
