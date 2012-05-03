// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    id: contactsPanel
    anchors.fill: parent
    signal contactClicked(variant contact)
    onContactClicked: telephony.showContactDetails(contact)

    Rectangle {
        id: searchArea
        height: 50
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        ContactsSearchCombo {
            id: contactsSearchBox
            anchors.verticalCenter: parent.verticalCenter
            height: 30
            width: 250

            leftIconSource: text ? "assets/cross.png" : "assets/search_icon.png"
            rightIconSource: "assets/call_icon.png"
            rightIconVisible: text.match("^[0-9+][0-9+-]*$") != null

            onLeftIconClicked: text = ""
            onRightIconClicked: telephony.startCallToNumber(text)
            onItemSelected: telephony.startCallToContact(item)
        }
        z: 1
    }

    Item {
        id: buttonsGroup
        anchors.top: searchArea.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 60

        TextCustom {
            anchors.centerIn: parent
            text: "Buttons will show in here"
        }
    }

    Rectangle {
        id: callLogHeader
        anchors.top: buttonsGroup.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        color: "lightGray"
        height: 30

        TextCustom {
            text: "Call Log"
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.margins: 5
        }
    }

    CallLogList {
        anchors.top: callLogHeader.bottom
        anchors.left: parent.left
        anchors.right: parent.right
    }
}


