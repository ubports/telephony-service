import QtQuick 2.0
import Ubuntu.Components 0.1

Button {
    id: button
    property alias text: label.text
    property alias icon: icon.source

    ItemStyle.class: "transparent"
    width: units.gu(6)
    height: units.gu(6)

    Image {
        id: icon
        anchors.top: parent.top
        anchors.topMargin: units.gu(1)
        anchors.horizontalCenter: parent.horizontalCenter
        height: units.gu(2)
        width: units.gu(2)
    }

    Label {
        id: label
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: units.gu(1)
        fontSize: "x-small"
        color: "#888888"
        width: paintedWidth
    }

    Rectangle {
        anchors.fill: parent
        color: "white"
        opacity: 0.4
        visible: button.pressed
    }
}
