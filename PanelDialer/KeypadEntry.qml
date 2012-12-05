import QtQuick 2.0
import Ubuntu.Components 0.1

FocusScope {
    id: keypadEntry

    property alias value: input.text
    property alias input: input

    height: units.gu(7.5)

    Image {
        id: divider

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        source: "../assets/dialer_top_number_bg.png"
    }

    // FIXME: check how to disable OSK
    TextInput {
        id: input

        anchors.left: parent.left
        anchors.leftMargin: units.gu(5)
        anchors.right: parent.right
        anchors.rightMargin: units.gu(2)
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: units.dp(-1)
        horizontalAlignment: TextInput.AlignRight
        text: ""
        font.pixelSize: units.dp(43)
        font.weight: Font.DemiBold
        font.family: "Ubuntu"
        color: "#464646"
        focus: true
        clip: true
        readOnly: true
    }
}
