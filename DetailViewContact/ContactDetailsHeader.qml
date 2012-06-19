import QtQuick 1.1
import QtMobility.contacts 1.1
import "../Widgets"

Item {
    id: contactDetailsHeader

    property variant contact: null
    property variant editable: false

    width: parent.width
    height: 82

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
        id: icon

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        width: 61
        height: width
        sourceSize.width: width
        fillMode: Image.PreserveAspectFit
        source: contact ? contact.avatar.imageUrl : ""
    }

    BorderImage {
        id: iconFrame

        source: "../Widgets/artwork/ListItemFrame.png"
        anchors.fill: icon
        anchors.bottomMargin: -1
        border.left: 3
        border.right: 3
        border.top: 3
        border.bottom: 3
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
    }

    Item {
        anchors.left: icon.right
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        height: childrenRect.height

        TextCustom {
            id: label

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            fontSize: "x-large"
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
            text: contact ? contact.name : "Unknown Contact"
        }

        TextCustom {
            id: sublabel

            anchors.top: label.bottom
            anchors.topMargin: 1
            anchors.left: parent.left
            anchors.right: parent.right
            fontSize: "medium"
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
            text: "A social update will show in here"
        }
    }
}
