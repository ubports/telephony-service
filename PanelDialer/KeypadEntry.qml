import QtQuick 2.0
import Ubuntu.Components 0.1

FocusScope {
    id: keypadEntry

    property alias value: input.text
    property alias input: input

    height: units.gu(6)

    TextInput {
        id: input

        anchors.left: parent.left
        anchors.leftMargin: units.gu(2)
        anchors.right: parent.right
        anchors.rightMargin: units.gu(1)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: units.dp(1)
        height: font.pixelSize
        horizontalAlignment: TextInput.AlignRight
        text: ""
        font.pixelSize: FontUtils.sizeToPixels("x-large")
        color: "#464646"
        focus: true
    }
}
