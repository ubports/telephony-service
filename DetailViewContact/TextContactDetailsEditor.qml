import QtQuick 1.1
import "../Widgets"
import "../fontUtils.js" as FontUtils

TextInput {
    font.pixelSize: FontUtils.sizeToPixels("medium")
    color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
    selectByMouse: true
}
