import QtQuick 1.1
import "../fontUtils.js" as Font

//FIXME: remove the whole file
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
