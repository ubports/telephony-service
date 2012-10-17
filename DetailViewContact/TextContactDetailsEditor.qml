import QtQuick 2.0
import "../fontUtils.js" as FontUtils

TextInput {
    font.pixelSize: FontUtils.sizeToPixels("medium")
    color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
    selectByMouse: true
}
