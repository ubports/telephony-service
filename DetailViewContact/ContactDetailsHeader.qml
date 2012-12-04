import QtQuick 2.0
import Ubuntu.Components 0.1
import "../Widgets" as LocalWidgets

FocusScope {
    id: contactDetailsHeader

    property variant contact: null
    property variant editable: false
    property alias contactNameValid: name.valid

    width: parent.width
    // ensure that there is equal padding at the top and bottom of labelBox
    height: editable ? Math.max(labelBox.height + labelBox.anchors.topMargin * 2, units.gu(10)) : units.gu(12)
    Behavior on height { LocalWidgets.StandardAnimation {}}

    function save() {
        name.save()
    }

    UbuntuShape {
        id: icon

        anchors.left: parent.left
        anchors.leftMargin: units.gu(2)
        anchors.top: parent.top
        anchors.topMargin: units.gu(2)
        width: units.gu(8)
        height: units.gu(8)

        image: Image {
            source: contact && contact.avatar != "" ? contact.avatar : "../assets/avatar_contacts_details.png"
        }
    }

    Item {
        id: labelBox

        anchors.left: icon.right
        anchors.leftMargin: units.gu(4)
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: units.gu(3)
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
