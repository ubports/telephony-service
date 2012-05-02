import QtQuick 1.1

Rectangle {
    width: 800
    height: 600

    Item {
        id: leftPane
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        Tabs {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
        }

        width: 250
    }

    Item {
        id: rightPane
        anchors.left: leftPane.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        Rectangle {
            anchors.fill: parent
            color: "#ebebeb"
        }
    }
}
