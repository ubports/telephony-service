import QtQuick 1.1
import "../fontUtils.js" as Font

ColoredButton {
    id: textButton
    property alias text: label.text
    property string fontSize: "medium"

    Text {
        id: label
        anchors.centerIn: parent
        font.pointSize: Font.sizeToPixels(textButton.fontSize);
    } // label
} // button
