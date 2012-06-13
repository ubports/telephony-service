import QtQuick 1.1
import QtMobility.contacts 1.1
import "../Widgets"

Item {
    id: contactDetailsHeader

    property variant contact: null

    signal editClicked
    signal saveClicked

    width: parent.width
    height: 100

    // FIXME: this function is used in two places, should be moved to one common place
    function contactName() {
        if (!contact)
            return "";
        if (contact.displayLabel != "")
            return contact.displayLabel
        if (contact.nickname.nickname != "")
            return contact.nickname.nickname;
        else if (contact.presence.nickname != "")
            return contact.presence.nickname;
    }

    function iconForState(state) {
        switch (state) {
        case Presence.Unknown:
        case Presence.Available:
        case Presence.Hidden:
        case Presence.Busy:
        case Presence.Away:
        case Presence.ExtendedAway:
        case Presence.Offline:
        default:
            return "../assets/icon_availability.png"
        }
    }

    function nameForState(state) {
        // FIXME: translate those strings
        switch (state) {
        case Presence.Available:
            return "Available";
        case Presence.Hidden:
            return "Hidden";
        case Presence.Busy:
            return "Busy";
        case Presence.Away:
        case Presence.ExtendedAway:
            return "Away";
        case Presence.Offline:
            return "Offline";
        case Presence.Unknown:
        default:
            return "Unknown";
        }
    }

    Image {
        id: avatar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 10
        source: {
            if (contact) {
                if (contact.avatar.imageUrl) {
                    if (contact.avatar.imageUrl.toString().length > 0) {
                        return contact.avatar.imageUrl;
                    }
                }
            }
            return "../assets/default_avatar.png";
        }
    }

    Column {
        anchors.left: avatar.right
        anchors.top: parent.top
        anchors.right: editButton.left
        anchors.margins: 10

        spacing: 10

        TextCustom {
            id: contactName
            anchors.left: parent.left
            anchors.right: parent.right
            fontSize: "x-large"
            text: contactDetailsHeader.contactName()
        }

        TextCustom {
            id: statusUpdate
            anchors.left: parent.left
            anchors.right: parent.right
            text: "A social update will show in here"
        }

        Row {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 5

            Image {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                fillMode: Image.PreserveAspectFit
                source: (contact) ? contactDetailsHeader.iconForState(contact.presence.state) : ""
            }

            TextCustom {
                anchors.verticalCenter: parent.verticalCenter
                text: (contact) ? nameForState(contact.presence.state) : ""
            }
        }
    }

    Button {
        id: editButton
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10
        text: "Edit"
        color: "gray"
        height: 30
        width: 70

        // FIXME: re-enable this button once contact modification is properly implemented
        visible: false

        onClicked: {
            if (text == "Edit") {
                text = "Save"
                contactDetailsHeader.editClicked()
            } else {
                text = "Edit"
                contactDetailsHeader.saveClicked()
            }
        }
    }
}
