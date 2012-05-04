import QtQuick 1.1
import "../fontUtils.js" as Font

Item {
    id: messages

    Rectangle {
        id: line
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: 1
        color: "gray"
    }
}
