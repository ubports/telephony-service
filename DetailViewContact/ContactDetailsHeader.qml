import QtQuick 1.1
import QtMobility.contacts 1.1
import "../Widgets"

Item {
    id: contactDetailsHeader

    property variant contact: null
    property variant editable: false

    width: parent.width
    height: (editable) ? name.height + 64 : 100

    // FIXME: this function is used in two places, should be moved to one common place
    function contactName() {
        if (!contact)
            return "";
        if (contact.displayLabel)
            return contact.displayLabel
        if (contact.nickname && contact.nickname.nickname)
            return contact.nickname.nickname;
        else if (contact.presence && contact.presence.nickname)
            return contact.presence.nickname;
        else return "";
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

    function save() {
        name.save()
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
        anchors.right: parent.right
        anchors.margins: 10

        spacing: 10

        NameContactDetails {
            id: name
            anchors.left: parent.left
            anchors.right: parent.right

            editable: header.editable
            detail: (contact) ? contact.name : undefined
        }

        TextCustom {
            id: statusUpdate
            anchors.left: parent.left
            anchors.right: parent.right
            text: "A social update will show in here"

            opacity: !editable ? 1.0 : 0.0
        }

        Row {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 5
            opacity: !editable ? 1.0 : 0.0

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
}
