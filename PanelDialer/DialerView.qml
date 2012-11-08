import QtQuick 2.0
import Ubuntu.Components 0.1
import "../Widgets" as LocalWidgets

LocalWidgets.TelephonyPage {
    title: "Dialer"
    anchors.fill: parent
    property string voicemailNumber: callManager.voicemailNumber
    property alias dialNumber: keypadEntry.value

    function isVoicemailActive() {
        return telephony.isVoicemailActive();
    }

    FocusScope {
        anchors.fill: parent
        focus: true

        Rectangle {
            id: background

            anchors.fill: parent
            color: "#3a3c41"
        }

        Item {
            width: keypad.width
            height: childrenRect.height

            anchors.centerIn: parent
            anchors.verticalCenterOffset: -units.gu(3)

            KeypadEntry {
                id: keypadEntry

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
            }

            CallButton {
                id: callButton
                objectName: "callButton"
                anchors.top: keypad.bottom
                anchors.topMargin: units.gu(2)
                color: "#c53e10"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: telephony.callNumber(keypadEntry.value)
            }

            Button {
                id: contactListButton
                objectName: contactListButton
                anchors.left: callButton.right
                anchors.verticalCenter: callButton.verticalCenter
                iconSource: "../assets/tab_icon_contacts_inactive.png"
                width: 48
                color: "#565656"
                ItemStyle.class: "dark-button"
                onClicked: {
                    telephony.switchToTab(telephony.contactDetails.tab)
                }
            }
        }
    }
}
