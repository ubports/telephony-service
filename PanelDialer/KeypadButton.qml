import QtQuick 2.0
import Ubuntu.Components 0.1

Button {
    id: button

    width: units.gu(11)
    height: units.gu(8)

    property alias label: labelItem.text
    property alias sublabel: sublabelItem.text
    property alias sublabelSize: sublabelItem.fontSize
    property alias iconSource: subImage.source
    property int keycode
    property bool isCorner: false
    property int corner
    ItemStyle.class: "transparent"

    UbuntuShape {
        id: shape
        anchors.centerIn: parent
        width: units.gu(11)
        height: units.gu(8)
        borderSource: pressed ? "../assets/dialer_pad_bg_pressed.sci" : "../assets/dialer_pad_bg.sci"
    }

    Label {
        id: labelItem

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenterOffset: -units.gu(0.5)
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: units.dp(43)
        font.weight: Font.Light
        color: "#464646"
        height: paintedHeight
        verticalAlignment: Text.AlignTop
        opacity: 0.9
    }

    Label {
        id: sublabelItem

        anchors.bottom: shape.bottom
        anchors.bottomMargin: units.dp(7)
        anchors.horizontalCenter: parent.horizontalCenter
        horizontalAlignment: Text.AlignHCenter
        fontSize: "x-small"
        color: "#a3a3a3"
    }

    Image {
        id: subImage
        visible: source != ""
        anchors.centerIn: sublabelItem
        opacity: 0.8
    }
}
