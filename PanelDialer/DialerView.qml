import QtQuick 2.0
import Ubuntu.Components 0.1
import "../Widgets" as LocalWidgets

LocalWidgets.TelephonyPage {
    title: "Dialer"
    showChromeBar: false
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

        LocalWidgets.Header {
            id: header
            text: title
        }

        KeypadEntry {
            id: keypadEntry

            anchors.top: header.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            focus: true
            Keys.forwardTo: [callButton]
        }

        Keypad {
            id: keypad

            anchors.top: keypadEntry.bottom
            onKeyPressed: keypadEntry.value += label
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: units.gu(3)
        }

        BorderImage {
            id: divider3

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: keypad.bottom
            anchors.topMargin: units.gu(2)
            source: "../assets/horizontal_divider.sci"
            height: units.dp(2)
        }

        CallButton {
            id: callButton
            objectName: "callButton"
            anchors.top: divider3.bottom
            anchors.topMargin: units.gu(2)
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: telephony.callNumber(keypadEntry.value)
            enabled: dialNumber != ""
        }

        CustomButton {
            id: contactListButton
            objectName: "contactListButton"
            anchors.right: callButton.left
            anchors.verticalCenter: callButton.verticalCenter
            anchors.rightMargin: units.gu(1)
            icon: "../assets/dialer_contacts.png"
            iconWidth: units.gu(4)
            iconHeight: units.gu(4)
            width: units.gu(7)
            height: units.gu(7)
            onClicked: {
                telephony.switchToTab(telephony.contactDetails.tab)
            }
        }

        CustomButton {
            id: backspace
            anchors.left: callButton.right
            anchors.verticalCenter: callButton.verticalCenter
            anchors.leftMargin: units.gu(1)
            width: units.gu(7)
            height: units.gu(7)
            icon: "../assets/dialer_backspace.png"
            iconWidth: units.gu(4)
            iconHeight: units.gu(4)

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
