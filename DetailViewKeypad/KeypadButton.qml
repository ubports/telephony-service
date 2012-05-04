import QtQuick 1.1
import "../Widgets"

AbstractButton {
    width: 62
    height: 52

    property alias label: labelItem.text
    property int keycode

    Rectangle {
        anchors.fill: parent
        border.color: "#e0e0e0"
        border.width: 1
        color: "#8d8d8d"
    }

    TextCustom {
        id: labelItem
        anchors.centerIn: parent
        fontSize: "x-large"
        color: "white"
    }
}
