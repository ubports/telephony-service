import QtQuick 1.1
import "../Widgets"

AbstractButton {
    id: button

    width: 64
    height: 48

    property alias label: labelItem.text
    property alias sublabel: sublabelItem.text
    property int keycode
    property bool isCorner: false
    property int corner

    Item {
        id: selectionOverlay

        anchors.fill: parent
        anchors.bottomMargin: 1
        visible: button.state == "pressed"

        BorderImage {
            anchors.fill: parent
            visible: button.isCorner && button.corner == Qt.BottomLeftCorner
            source: "../assets/keypad_select_bottom_left.png"
            border.left: 14
            border.bottom: 10
            horizontalTileMode: BorderImage.Stretch
            verticalTileMode: BorderImage.Stretch
        }

        BorderImage {
            anchors.fill: parent
            visible: button.isCorner && button.corner == Qt.BottomRightCorner
            source: "../assets/keypad_select_bottom_right.png"
            border.left: 14
            border.bottom: 10
            horizontalTileMode: BorderImage.Stretch
            verticalTileMode: BorderImage.Stretch
        }

        Image {
            anchors.fill: parent
            visible: !button.isCorner
            source: "../assets/keypad_select_everywhere_else.png"
        }
    }

    TextCustom {
        id: labelItem

        anchors.centerIn: parent
        anchors.verticalCenterOffset: -3
        anchors.horizontalCenterOffset: 1
        horizontalAlignment: Text.AlignHCenter
        fontSize: "large"
        color: "#e3e5e8"
        style: Text.Sunken
        styleColor: "#2e2e2e"
    }

    TextCustom {
        id: sublabelItem

        anchors.top: labelItem.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 1
        horizontalAlignment: Text.AlignHCenter
        fontSize: "x-small"
        color: "#e3e5e8"
        style: Text.Sunken
        styleColor: "#2e2e2e"
    }
}
