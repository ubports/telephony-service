import QtQuick 2.0

Item {
    id: keyboardRect
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    height: Qt.inputMethod.visible ? Qt.inputMethod.keyboardRectangle.height : 0

    Behavior on height {
        StandardAnimation { }
    }
}
