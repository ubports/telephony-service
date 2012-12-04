import QtQuick 2.0
import Ubuntu.Components 0.1

Button {
    width: units.gu(20)
    height: units.gu(8)
    // FIXME: waiting on #1072733
    //iconSource: "../assets/dialer_call.png"
    Image {
        anchors.centerIn: parent
        width: units.gu(4)
        height: units.gu(4)
        source: "../assets/dialer_call.png"
        fillMode: Image.PreserveAspectFit
        z: 1
    }
    color: "#1d9d1c"
    ItemStyle.class: "button"
}
