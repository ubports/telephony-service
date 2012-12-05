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
        
        // we dont receive focus to avoid osk to appear
        activeFocusOnPress: false
        focus: false
        cursorVisible: true
        clip: true

        // force cursor to be always visible
        onCursorVisibleChanged: {
            if (!cursorVisible)
                cursorVisible = true
        }
    }

    MouseArea {
        anchors.fill: parent
        property bool held: false
        onPressAndHold: {
            if (input.text != "") {
                held = true
                input.selectAll()
                input.copy()
            } else {
                input.paste()
            }
        }
        onReleased: {
            if(held) {
                input.deselect()
                held = false
            }

        }
    }

    Label {
        id: hint
        visible: input.text == ""
        anchors.centerIn: input
        text: "Enter a number"
        font.pixelSize: units.dp(20)
        font.weight: Font.DemiBold
        font.family: "Ubuntu"
        color: "#464646"
        opacity: 0.25
    }
}
