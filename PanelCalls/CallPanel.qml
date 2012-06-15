import QtQuick 1.1
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
//        hint: "Quick dial"

        onLeftIconClicked: text = ""
        onRightIconClicked: {
            telephony.startCallToNumber(text);
            text = ""
        }
        onContactSelected: {
            telephony.startCallToContact(contact, number)
            text = ""
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
            anchors.left: parent.left
            anchors.right: parent.right

            isIcon: true
            iconSource: selected ? "../assets/call_icon_keypad_active.png" : "../assets/call_icon_keypad_inactive.png"
            text: "Keypad"
            onClicked: telephony.showDial();
        }

        ListItem {
            anchors.left: parent.left
            anchors.right: parent.right

            isIcon: true
            iconSource: selected ? "../assets/call_icon_voicemail_active.png" : "../assets/call_icon_voicemail_inactive.png"
            text: "Voicemail"
        }

        ListItem {
            anchors.left: parent.left
            anchors.right: parent.right

            isLast: true
            isIcon: true
            iconSource: selected ? "../assets/call_icon_call_log_active.png" : "../assets/call_icon_call_log_inactive.png"
            text: "Call Log"
            onClicked: telephony.showCallLog();
        }
    }
}


