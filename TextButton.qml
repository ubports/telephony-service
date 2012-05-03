import QtQuick 1.1

ColoredButton {
    id: textButton
    property alias text: label.text

    Text {
        id: label
        anchors.centerIn: parent
        font.pointSize: 18;
    } // label
} // button
