import QtQuick 1.1
import "../Widgets"

Column {
    id: editor
    property variant detail
    property alias prefix: prefix.text
    property alias firstName: firstName.text
    property alias middleName: middleName.text
    property alias lastName: lastName.text
    property alias suffix: suffix.text

    TextCustom {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 4
        anchors.leftMargin: 4
        fontSize: "small"
        text: "Prefix"
    }

    Rectangle {
        border.width: 1
        border.color: "black"
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height + 4

        TextInput {
            id: prefix
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 4
            anchors.leftMargin: 4
            text: (detail) ? detail.prefix : ""
            font.pixelSize: 20
        }
    }

    TextCustom {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 4
        anchors.leftMargin: 4
        fontSize: "small"
        text: "First name"
    }

    Rectangle {
        border.width: 1
        border.color: "black"
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height + 4

        TextInput {
            id: firstName
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 4
            anchors.leftMargin: 4
            text: (detail) ? detail.firstName : ""
            font.pixelSize: 20
        }
    }

    TextCustom {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 4
        anchors.leftMargin: 4
        fontSize: "small"
        text: "Middle name"
    }

    Rectangle {
        border.width: 1
        border.color: "black"
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height + 4

        TextInput {
            id: middleName
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 4
            anchors.leftMargin: 4
            text: (detail) ? detail.middleName : ""
            font.pixelSize: 20
        }
    }

    TextCustom {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 4
        anchors.leftMargin: 4
        fontSize: "small"
        text: "Last name"
    }

    Rectangle {
        border.width: 1
        border.color: "black"
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height + 4

        TextInput {
            id: lastName
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 4
            anchors.leftMargin: 4
            text: (detail) ? detail.lastName : ""
            font.pixelSize: 20
        }
    }

    TextCustom {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 4
        anchors.leftMargin: 4
        fontSize: "small"
        text: "Suffix"
    }

    Rectangle {
        border.width: 1
        border.color: "black"
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height + 4

        TextInput {
            id: suffix
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 4
            anchors.leftMargin: 4
            text: (detail) ? detail.suffix : ""
            font.pixelSize: 20
        }
    }
}
