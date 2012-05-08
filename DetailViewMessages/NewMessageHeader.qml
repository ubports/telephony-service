import QtQuick 1.1
import "../fontUtils.js" as Font
import "../Widgets"

Item {
    width: 575
    height: 100

    property variant contact

    //FIXME: use maybe ContactSearchCombo
    SearchEntry {
        id: toField
        height: 30
        anchors {
            left: image.right
            leftMargin: 20
            right: parent.right
            rightMargin: 20
            verticalCenter: parent.verticalCenter
        }
    }

    // this is a placeholder for image
    Rectangle {
        id: image
        color: "white"
        width: height
        anchors {
            left: parent.left
            leftMargin: 20
            top: parent.top
            bottom: parent.bottom
            topMargin: 20
            bottomMargin: 20
        }
    }

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
