import QtQuick 1.1

AbstractButton {

    property alias color: background.color
    property color borderColor
    property int borderWidth
    property real radius: 3.0

    Rectangle {
        id: background
        radius: parent.radius
        width: parent.width
        height: parent.height
        border.color: parent.borderColor
        border.width: parent.borderWidth
    } // background

}
