import QtQuick 1.1
import "../Widgets"

FocusScope {
    id: contactDetailsHeader

    property variant contact: null
    property variant editable: false
    property alias contactNameValid: name.valid

    width: parent.width
    // ensure that there is equal padding at the top and bottom of labelBox
    height: editable ? Math.max(labelBox.height + labelBox.anchors.topMargin * 2, 82) : 82
    Behavior on height {StandardAnimation {}}

    function save() {
        name.save()
    }

    Image {
        id: icon

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        width: 61
        height: width
        sourceSize.width: width
        fillMode: Image.PreserveAspectFit
        source: (contact && contact.avatar != "") ? contact.avatar : "../assets/avatar_contacts_details.png"
        onStatusChanged: if (status == Image.Error) source = "../assets/avatar_contacts_details.png"
        asynchronous: true
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
        id: labelBox

        anchors.left: icon.right
        anchors.leftMargin: 10
        anchors.right: parent.right
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
            focus: true
        }
    }
}
