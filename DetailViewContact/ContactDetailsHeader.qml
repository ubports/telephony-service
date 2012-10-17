import QtQuick 2.0
import "../Widgets" as LocalWidgets

FocusScope {
    id: contactDetailsHeader

    property variant contact: null
    property variant editable: false
    property alias contactNameValid: name.valid

    width: parent.width
    // ensure that there is equal padding at the top and bottom of labelBox
    height: editable ? Math.max(labelBox.height + labelBox.anchors.topMargin * 2, 82) : 82
    Behavior on height { LocalWidgets.StandardAnimation {}}

    function save() {
        name.save()
    }

    LocalWidgets.FramedImage {
        id: icon

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        width: 61
        height: width
        source: contact ? contact.avatar : fallbackSource
        fallbackSource: "../assets/avatar_contacts_details.png"
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
