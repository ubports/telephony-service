import QtQuick 1.1

ColoredButton {
    id: button
    property alias icon: icon.source
    property alias text: label.text
    property alias iconWidth: icon.width
    property alias iconHeight: icon.height

    /**
     * The location of the text relative to the icon.
     * top, bottom, left or right.
     */
    property string iconLocation: "left"

    Image {
        id: icon
        fillMode: Image.PreserveAspectFit
        anchors.margins: 10
    }

    Text {
        id: label
        anchors.margins: 10
        font.pointSize: 18
    }

    function alignIconText() {
        if (iconLocation=="top") {
            icon.anchors.top = button.top;
            icon.anchors.horizontalCenter = button.horizontalCenter;
            label.anchors.top = icon.bottom;
            label.anchors.horizontalCenter = button.horizontalCenter;
        } else if (iconLocation=="bottom") {
            icon.anchors.bottom = button.bottom;
            icon.anchors.horizontalCenter = button.horizontalCenter;
            label.anchors.bottom = icon.top;
            label.anchors.horizontalCenter = button.horizontalCenter;
        } else if (iconLocation=="right") {
            icon.height = button.height - 10;
            icon.anchors.right = button.right;
            icon.anchors.verticalCenter = button.verticalCenter;
            label.anchors.right = icon.left;
            label.anchors.verticalCenter = button.verticalCenter;
        } else if (iconLocation=="left") {
            icon.height = button.height - 10;
            icon.anchors.left = button.left;
            icon.anchors.verticalCenter = button.verticalCenter;
            label.anchors.left = icon.right;
            label.anchors.verticalCenter = button.verticalCenter;
        } // if textlocation
    } // alignIconText

    Component.onCompleted: alignIconText()
}

