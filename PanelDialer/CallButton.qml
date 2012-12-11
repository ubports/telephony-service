import QtQuick 2.0
import Ubuntu.Components 0.1

AbstractButton {
    id: button
    width: units.gu(20)
    height: units.gu(8)
    // FIXME: waiting on #1072733
    //iconSource: "../assets/dialer_call.png"

    UbuntuShape {
        anchors.fill: parent
        color: button.pressed ? "#cd3804" : "#dd4814"
        radius: "medium"
    }

    Image {
        anchors.centerIn: parent
        width: units.gu(4)
        height: units.gu(4)
        source: "../assets/dialer_call.png"
        fillMode: Image.PreserveAspectFit
        z: 1

    }
}
