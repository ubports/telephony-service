import QtQuick 1.1
import "../fontUtils.js" as Font
import "../Widgets"

Item {
    id: header
    width: 575
    height: 100

    signal contactSelected(variant contact, string number)
    signal numberSelected(string number)

    ContactsSearchCombo {
        id: contactsSearchEntry
        height: 30
        anchors {
            left: image.right
            leftMargin: 20
            right: parent.right
            rightMargin: 20
            verticalCenter: parent.verticalCenter
        }
        leftIconSource: "../assets/empty.png"
        onLeftIconClicked: text = ""

        rightIconSource: "../assets/icon_message_grey.png"
        rightIconVisible: text.match("^[0-9+][0-9+-]*$") != null

        onRightIconClicked: {
            header.numberSelected(text)
        }

        onContactSelected: {
            header.contactSelected(contact, number);
        }

        TextCustom {
            id: toText
            text: "To:"
            fontSize: "large"
            color: "white"
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: 10
            }
        }
    }

    // this is a placeholder for image
    Rectangle {
        id: image
        color: "white"
        width: height
        anchors {
            left: parent.left
            leftMargin: 20
            top: parent.top
            bottom: parent.bottom
            topMargin: 20
            bottomMargin: 20
        }
    }

    Rectangle {
        id: line
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: 1
        color: "gray"
    }
}
