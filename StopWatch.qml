import QtQuick 1.0
import "fontUtils.js" as Font

Rectangle {
    property string time: ""
    radius: 4
    color: "#3a3a3a"
    height: count.paintedHeight + 8 * 2
    width: count.paintedWidth + 22 * 2

    Text {
        id: count
        anchors.centerIn: parent
        color: "white"
        text: time
        font.pixelSize: Font.sizeToPixels("x-large")
    }
}
