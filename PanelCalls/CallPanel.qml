import QtQuick 2.0
import TelephonyApp 0.1
import "../Widgets" as LocalWidgets
import Ubuntu.Components.ListItems 0.1 as ListItem

Item {
    id: contactsPanel
    anchors.fill: parent
    signal contactClicked(variant contact)
    onContactClicked: telephony.showContactDetails(contact)

    LocalWidgets.ContactsSearchCombo {
        id: contactsSearchBox

        anchors.top: parent.top
        anchors.topMargin: units.dp(10)
        anchors.left: parent.left
        anchors.leftMargin: units.dp(10)
        anchors.right: parent.right
        anchors.rightMargin: units.dp(10)

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
        onActivateFirstResult: {
            if (text.match("^[0-9+][0-9+-]*$")) {
                telephony.callNumber(text);
                text = "";
            }
        }
        z: 1
    }

    Column {
        id: buttonsGroup
        anchors.top: contactsSearchBox.bottom
        anchors.topMargin: units.dp(10)
        anchors.left: parent.left
        anchors.right: parent.right

        ListItem.ThinDivider {}

        ListItem.Standard {
            anchors.left: parent.left
            anchors.right: parent.right
            height: units.dp(30)
            __leftIconMargin: units.dp(19)
            __rightIconMargin: units.dp(14)

            function getIconSource() {
                if (callManager.hasCalls && !telephony.isVoicemailActive()) {
                    return selected ? "../assets/call_icon_livecall_active.png" : "../assets/call_icon_livecall_inactive.png"
                } else {
                    return selected ? "../assets/call_icon_keypad_active.png" : "../assets/call_icon_keypad_inactive.png"
                }
            }

            icon: Qt.resolvedUrl(getIconSource())
            iconFrame: false
            text: callManager.hasCalls && !telephony.isVoicemailActive() ? "On Call" : "Keypad"
            onClicked: callManager.hasCalls && !telephony.isVoicemailActive() ? telephony.showLiveCall() : telephony.showKeypad();
            selected: telephony.liveCall.loaded || telephony.keypad.loaded
        }

        ListItem.Standard {
            anchors.left: parent.left
            anchors.right: parent.right
            height: units.dp(30)
            __leftIconMargin: units.dp(19)
            __rightIconMargin: units.dp(14)
            visible: callManager.voicemailNumber != ""

            icon: Qt.resolvedUrl(selected ? "../assets/call_icon_voicemail_active.png" : "../assets/call_icon_voicemail_inactive.png")
            iconFrame: false
            text: "Voicemail"

            onClicked: telephony.showVoicemail()
            selected: telephony.voicemail.loaded
        }

        ListItem.Standard {
            anchors.left: parent.left
            anchors.right: parent.right
            height: units.dp(30)
            __leftIconMargin: units.dp(19)
            __rightIconMargin: units.dp(14)

            icon: Qt.resolvedUrl(selected ? "../assets/call_icon_call_log_active.png" : "../assets/call_icon_call_log_inactive.png")
            iconFrame: false
            text: "Call Log"
            onClicked: telephony.showCallLog();
            selected: telephony.callLog.loaded
        }
    }
}


