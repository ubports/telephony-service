import QtQuick 2.0
import Ubuntu.Components 0.1

AbstractButton {
    width: image.paintedWidth
    height: image.paintedHeight

    Image {
        id: image
        source: "../assets/keypad_backspace.png"
        width: units.gu(3)
        height: units.gu(2)
    }
}
