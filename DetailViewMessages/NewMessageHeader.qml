import QtQuick 1.1
import "../fontUtils.js" as Font
import "../Widgets"
import Ubuntu.Components 0.1

BaseMessageHeader {
    id: header

    signal contactSelected(variant contact, string number)
    signal numberSelected(string number)

    property alias text: contactsSearchEntry.text

    ContactsSearchCombo {
        id: contactsSearchEntry

        anchors {
            left: parent.left
            leftMargin: 10
            right: parent.right
            rightMargin: 10
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: -1
        }
        leftIconSource: "../assets/empty.png"
        onLeftIconClicked: text = ""

        rightIconSource: "../assets/icon_message_grey.png"
        rightIconVisible: text.match("^[0-9+][0-9+-]*$") != null
        focus: true

        onRightIconClicked: {
            header.numberSelected(text)
        }

        onContactSelected: {
            header.contactSelected(contact, number);
        }

        TextCustom {
            id: toText
            text: "To:"
            fontSize: "small"
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: 10
            }
        }
    }
}
