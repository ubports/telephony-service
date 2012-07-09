import QtQuick 1.1
import TelephonyApp 0.1
import "../Widgets"

Item {
    id: contactsPanel
    anchors.fill: parent
    signal contactClicked(variant contact)
    onContactClicked: telephony.showContactDetails(contact)

    // FIXME: port to use the QtMobility contacts model
    ContactsSearchCombo {
        id: contactsSearchBox

        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10

        rightIconSource: "../assets/call_icon.png"
        rightIconVisible: text.match("^[0-9+][0-9+-]*$") != null
        hint: "Quick dial"

        onLeftIconClicked: text = ""
        onRightIconClicked: {
            telephony.startCallToNumber(text);
            text = "";
        }
        onContactSelected: {
            telephony.callNumber(number);
            text = "";
        }
        z: 1
    }

    Column {
        id: buttonsGroup
        anchors.top: contactsSearchBox.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.right: parent.right

        ListItem {

            function getIconSource() {
                if (callManager.hasCalls && !telephony.isVoicemailActive()) {
                    return selected ? "../assets/call_icon_livecall_active.png" : "../assets/call_icon_livecall_inactive.png"
                } else {
                    return selected ? "../assets/call_icon_keypad_active.png" : "../assets/call_icon_keypad_inactive.png"
                }
            }

            anchors.left: parent.left
            anchors.right: parent.right

            topSeparator: true
            isIcon: true
            iconSource: getIconSource()
            text: callManager.hasCalls && !telephony.isVoicemailActive() ? "On Call" : "Keypad"
            onClicked: callManager.hasCalls && !telephony.isVoicemailActive() ? telephony.showLiveCall() : telephony.showKeypad();
            selected: telephony.liveCall.loaded || telephony.keypad.loaded
        }

        ListItem {
            anchors.left: parent.left
            anchors.right: parent.right

            isIcon: true
            iconSource: selected ? "../assets/call_icon_voicemail_active.png" : "../assets/call_icon_voicemail_inactive.png"
            text: "Voicemail"
            visible: callManager.getVoicemailNumber() != ""
            onClicked: telephony.showVoicemail()
            selected: telephony.voicemail.loaded
        }

        ListItem {
            anchors.left: parent.left
            anchors.right: parent.right

            isIcon: true
            iconSource: selected ? "../assets/call_icon_call_log_active.png" : "../assets/call_icon_call_log_inactive.png"
            text: "Call Log"
            onClicked: telephony.showCallLog();
            selected: telephony.callLog.loaded
        }
    }
}


