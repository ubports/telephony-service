import QtQuick 2.0
import Ubuntu.Components 0.1

Button {
    // FIXME: waiting on #1072733
    //iconSource: "../assets/dialer_call.png"
    property string icon
    property int iconWidth
    property int iconHeight

    Image {
        anchors.fill: parent
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        source: pressed ? "../assets/dialer_pad_bg_pressed.png" : "../assets/dialer_pad_bg.png"
        fillMode: Image.PreserveAspectFit
    }

    Image {
        anchors.centerIn: parent
        width: iconWidth
        height: iconHeight
        source: icon
        fillMode: Image.PreserveAspectFit
        z: 1
    }
    ItemStyle.class: "transparent"
}
