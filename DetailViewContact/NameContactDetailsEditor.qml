import QtQuick 2
import "../Widgets" as LocalWidgets
import Ubuntu.Components 0.1

FocusScope {
    id: editor

    property variant detail
    property bool expanded: false

    property alias prefix: prefix.value
    property alias firstName: firstName.value
    property alias middleName: middleName.value
    property alias lastName: lastName.value
    property alias suffix: suffix.value

    height: fields.height + 2 * fields.anchors.topMargin

    BorderImage {
        id: background

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: fields.height + 2 * fields.anchors.topMargin
        Behavior on height { LocalWidgets.StandardAnimation {}}

        source: "../assets/edit_contact_mode_box.png"
        border {left: 1; right: 1; top: 2; bottom: 0}
    }

    Column {
        id: fields

        anchors.top: background.top
        anchors.topMargin: 5
        anchors.left: background.left
        anchors.leftMargin: 10
        anchors.right: background.right
        anchors.rightMargin: 10
        spacing: 2

        move: Transition { LocalWidgets.StandardAnimation {property: "y"}}

        EditBoxName {
            id: prefix

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.prefix : ""
            description: "Prefix"
            visible: editor.expanded
        }

        EditBoxName {
            id: firstName

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.firstName : ""
            description: "First"
            focus: true
        }

        EditBoxName {
            id: middleName

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.middleName : ""
            description: "Middle"
            visible: editor.expanded
        }

        EditBoxName {
            id: lastName

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.lastName : ""
            description: "Last"
        }

        EditBoxName {
            id: suffix

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.suffix : ""
            description: "Suffix"
            visible: editor.expanded
        }
    }

    AbstractButton {
        id: expandButton

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: 10
        width: 20

        Image {
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.right: parent.right
            source: editor.expanded ? "../assets/edit_contact_dropup_arrow.png" : "../assets/edit_contact_dropdown_arrow.png"
        }

        onClicked: editor.expanded = !editor.expanded
    }
}

