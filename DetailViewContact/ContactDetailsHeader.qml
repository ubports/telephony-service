import QtQuick 1.1
import "../Widgets"

Item {
    id: contactDetailsHeader

    property variant contact: null
    property variant editable: false

    width: parent.width
    height: editable ? name.height : 82

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

    function save() {
        name.save()
    }

    Image {
        id: icon

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: (editable) ? undefined : parent.verticalCenter
        anchors.top: (editable) ? parent.top : undefined
        anchors.topMargin: (editable) ? 16 : 0
        width: 61
        height: width
        sourceSize.width: width
        fillMode: Image.PreserveAspectFit
        source: contact ? contact.avatar : ""
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

        NameContactDetails {
            id: name
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            editable: header.editable
            detail: (contact) ? contact.name : undefined
        }

        TextCustom {
            id: sublabel

            anchors.top: name.bottom
            anchors.topMargin: 1
            anchors.left: parent.left
            anchors.right: parent.right
            fontSize: "medium"
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
            text: "A social update will show in here"
            opacity: !editable ? 1.0 : 0.0
        }
    }
}
