import QtQuick 1.1
import "../Widgets"

Item {
    id: contactsPanel
    anchors.fill: parent
    signal contactClicked(variant contact)
    onContactClicked: telephony.showContactDetails(contact)

    TextCustom {
        id: hint
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.margins: 5
        anchors.leftMargin: 8
        height: paintedHeight

        text: "Quick Dial"
    }

    Rectangle {
        id: searchArea
        height: 30
        anchors.top: hint.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5

        ContactsSearchCombo {
            id: contactsSearchBox
            anchors.fill: parent

            leftIconSource: text ? "../assets/cross.png" : "../assets/search_icon.png"
            rightIconSource: "../assets/call_icon.png"
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
        anchors.margins: 5
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
        anchors.margins: 5
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
        anchors.margins: 5
    }
}


