import QtQuick 1.1
import "../Widgets"

Item {
    height: 64
    width: parent.width
    signal clicked(variant contact)

    // FIXME: this function is used in two places, should be moved to one common place
    function contactName() {
        if (!contact)
            return "";
        if (contact.displayLabel != "")
            return contact.displayLabel
        else if (contact.nickname.nickname != "")
            return contact.nickname.nickname;
        else if (contact.presence.nickname != "")
            return contact.presence.nickname;
    }

    Rectangle {
        height: 1
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: "black"
    }
    Image {
        id: photoItem
        source: (contact && contact.avatar.imageUrl && (contact.avatar.imageUrl.toString().length > 0))
                ? contact.avatar.imageUrl : "../assets/default_avatar.png"
        width: 56
        height: width
        sourceSize.width: width
        sourceSize.height: height
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.verticalCenter: parent.verticalCenter
    }

    TextCustom {
        id: displayNameItem
        anchors.top: parent.top
        anchors.topMargin: 1
        anchors.left: photoItem.right
        anchors.leftMargin: 2
        fontSize: "medium"
        text: contactName()
    }

    TextCustom {
        id: emailItem
        anchors.top: displayNameItem.bottom
        anchors.topMargin: 2
        anchors.left: photoItem.right
        anchors.leftMargin: 2
        fontSize: "small"
        text: (contact && contact.emailAddress) ? contact.emailAddress.emailAddress : ""
    }

    TextCustom {
        id: phoneItem
        anchors.top: emailItem.bottom
        anchors.topMargin: 2
        anchors.left: photoItem.right
        anchors.leftMargin: 2
        fontSize: "small"
        text: (contact && contact.phoneNumber) ? contact.phoneNumber.number : ""
    }

    MouseArea {
        anchors.fill: parent
        onClicked: parent.clicked(contact)
    }
}
