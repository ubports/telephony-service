import QtQuick 1.1
import "fontUtils.js" as Font

Rectangle {
    id: callLog
    width: parent.width
    height: callLogView.height

    ListView {
        id: callLogView
        height: count * 64
        anchors.fill: parent
        interactive: false
        model: calllog
        delegate: CallLogDelegate {}
        section.property: "displayName"
    }
}
