import QtQuick 2.0
import Ubuntu.Components 0.1

Button {
    id: button

    width: units.gu(11)
    height: units.gu(8)

    property alias label: labelItem.text
    property alias sublabel: sublabelItem.text
    property int keycode
    property bool isCorner: false
    property int corner
    color: "transparent"

    TextCustom {
        id: labelItem

        anchors.centerIn: parent
        anchors.verticalCenterOffset: -units.gu(0.5)
        anchors.horizontalCenterOffset: units.dp(1)
        horizontalAlignment: Text.AlignHCenter
        fontSize: "x-large"
        color: "#464646"
        z: 1
    }

    TextCustom {
        id: sublabelItem

        anchors.top: labelItem.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: units.dp(1)
        horizontalAlignment: Text.AlignHCenter
        fontSize: "x-small"
        color: "#a3a3a3"
        z: 1
    }
}
