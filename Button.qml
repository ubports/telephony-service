import QtQuick 1.1

ColoredButton {
    property alias icon: icon.source
    property alias text: label.text

    property int verticalMargin: 1

    /**
     * The location of the text relative to the icon.
     * top, bottom, left or right.
     */
    property string textLocation: "top"
/*
    Image {
        anchors.centerIn: parent
        id: icon
        height: parent.height-(2*verticalMargin)
        fillMode: Image.PreserveAspectFit
    } // icon

    Text {
        id: label
        anchors.centerIn: parent
        font.pointSize: 18;
    } // label
*/

    Image {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        id: icon
        height: parent.height-(2*verticalMargin)
        fillMode: Image.PreserveAspectFit
        anchors.leftMargin: 10
    }

    Text {
        id: label
        anchors.leftMargin: 10
        anchors.left: icon.right
        anchors.verticalCenter: parent.verticalCenter
        font.pointSize: 18;
    }

}

