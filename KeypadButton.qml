import QtQuick 1.1

Item {
    width: 62
    height: 52

    property alias label: labelItem.text
    
    Rectangle {
        anchors.fill: parent
        border.color: "#e0e0e0"
        border.width: 1
        color: "#8d8d8d"
    }

    Text {
        id: labelItem
        anchors.centerIn: parent
    }
}
