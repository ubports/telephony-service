import QtQuick 1.1
import "../Widgets"

AbstractButton {
    width: image.paintedWidth
    height: image.paintedHeight

    Image {
        id: image
        source: "../assets/keypad_backspace.png"
        width: 22
        height: 16
    }
}
