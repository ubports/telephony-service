import QtQuick 2.0
import Ubuntu.Components 0.1
import "../Widgets" as LocalWidgets

LocalWidgets.TelephonyPage {
    title: "Dialer"
    anchors.fill: parent
    property string voicemailNumber: callManager.voicemailNumber
    property alias dialNumber: keypadEntry.value
    property alias input: keypadEntry.input

    function isVoicemailActive() {
        return telephony.isVoicemailActive();
    }

    FocusScope {
        anchors.fill: parent
        focus: true

        Image {
            id: divider

            anchors.top: parent.top
            source: "../assets/section_divider.png"
        }

        KeypadEntry {
            id: keypadEntry

            anchors.top: divider.bottom
            anchors.left: keypad.left
            anchors.right: keypad.right
            anchors.leftMargin: units.dp(-2)
            anchors.rightMargin: units.dp(-2)
            focus: true
            Keys.forwardTo: [callButton]
        }

        Keypad {
            id: keypad

            anchors.top: keypadEntry.bottom
            onKeyPressed: keypadEntry.value += label
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: units.dp(10)
        }

        CallButton {
            id: callButton
            objectName: "callButton"
            anchors.top: keypad.bottom
            anchors.topMargin: units.gu(2)
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: telephony.callNumber(keypadEntry.value)
        }

        Button {
            id: contactListButton
            objectName: "contactListButton"
            anchors.right: callButton.left
            anchors.verticalCenter: callButton.verticalCenter
            anchors.rightMargin: units.dp(6)
            iconSource: "../assets/tab_icon_contacts_inactive.png"
            width: units.gu(7)
            height: units.gu(7)
            color: "transparent"
            onClicked: {
                telephony.switchToTab(telephony.contactDetails.tab)
            }
        }

        Button {
            id: backspace
            anchors.left: callButton.right
            anchors.verticalCenter: callButton.verticalCenter
            anchors.leftMargin: units.dp(6)
            width: units.gu(7)
            height: units.gu(7)
            iconSource: "../assets/keypad_backspace.png"
            color: "transparent"

            onClicked:  {
                if (input.cursorPosition != 0)  {
                    var position = input.cursorPosition;
                    input.text = input.text.slice(0, input.cursorPosition - 1) + input.text.slice(input.cursorPosition);
                    input.cursorPosition = position - 1;
                }
            }
        }
    }
}
