import QtQuick 2.0
import Ubuntu.Components 0.1

Button {
    // FIXME: waiting on #1072733
    //iconSource: "../assets/dialer_call.png"
    property string icon
    property int iconWidth
    property int iconHeight

    UbuntuShape {
        id: shape
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        borderSource: pressed ? "../assets/dialer_pad_bg_pressed.sci" : "../assets/dialer_pad_bg.sci"
    }

    Image {
        anchors.centerIn: parent
        width: iconWidth
        height: iconHeight
        source: icon
        fillMode: Image.PreserveAspectFit
    }
    ItemStyle.class: "transparent"
}
