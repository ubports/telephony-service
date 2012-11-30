import QtQuick 2.0
import Ubuntu.Components 0.1
import "../Widgets" as LocalWidgets

BaseMessageHeader {
    id: header

    property variant contact
    property string number
    property string title

    height: icon.height + units.gu(4)

    UbuntuShape {
        id: icon

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: units.gu(2)

        width: units.gu(6)
        height: units.gu(6)

        image: Image {
            source: contact && contact.avatar != "" ? contact.avatar : "../assets/avatar_messaging.png"
            fillMode: Image.PreserveAspectCrop
        }
    }

    Label {
        id: name

        anchors.left: icon.right
        anchors.leftMargin: units.gu(2)
        anchors.right: parent.right
        anchors.rightMargin: units.gu(1)
        anchors.verticalCenter: parent.verticalCenter
        fontSize: "medium"
        elide: Text.ElideRight
        color: "#333333"
        opacity: 0.6
        text: contact ? contact.displayLabel : title
    }

    BorderImage {
        id: separator
        anchors.right: contactDetailsButton.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: units.gu(2)
        source: "../assets/contacts_vertical_divider.sci"
        width: units.dp(2)
    }

    Button {
        id: contactDetailsButton
        ItemStyle.class: "transparent-button"
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: units.gu(2)
        width: units.gu(3)
        height: units.gu(3)

        iconSource: "../assets/tab_icon_contacts_inactive.png"
        onClicked: telephony.showContactDetails(contact, true)
    }
}
