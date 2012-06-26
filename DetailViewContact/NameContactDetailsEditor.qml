import QtQuick 1.1

Item {
    id: editor

    property variant detail
    property bool expanded: false

    property alias prefix: prefix.value
    property alias firstName: firstName.value
    property alias middleName: middleName.value
    property alias lastName: lastName.value
    property alias suffix: suffix.value

    height: fields.height

    Column {
        id: fields

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 2

        EditBoxName {
            id: prefix

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.prefix : ""
            description: "Prefix"
        }

        EditBoxName {
            id: firstName

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.firstName : ""
            description: "First"
        }

        EditBoxName {
            id: middleName

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.middleName : ""
            description: "Middle"
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
        }
    }
}

