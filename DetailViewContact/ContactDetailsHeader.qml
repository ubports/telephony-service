import QtQuick 1.1
import "../Widgets"

Item {
    id: contactDetailsHeader

    property variant contact: null

    signal editClicked
    signal saveClicked

    width: parent.width
    height: 100

    Image {
        id: avatar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 10
        source: (contact) ? "../dummydata/" + contact.photo : ""
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
            text: (contact) ? contact.displayName : ""
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
                source: "../assets/icon_availability.png"
            }

            TextCustom {
                anchors.verticalCenter: parent.verticalCenter
                text: "Available"
            }
        }
    }

    TextButton {
        id: editButton
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10
        text: "Edit"
        color: "gray"
        radius: 5

        height: 30
        width: 70

        onClicked: {
            if (text == "Edit") {
                text = "Save"
                contactDetailsHeader.editClicked()
            }
            else {
                text = "Edit"
                contactDetailsHeader.saveClicked()
            }
        }
    }
}
