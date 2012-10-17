import QtQuick 1.1
import Ubuntu.Components 0.1

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
