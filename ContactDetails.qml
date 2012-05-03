// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    id: contactDetails
    width: 400
    height: 600

    states: [
        State {
            name: "edit"
            PropertyChanges {
                target: editButton
                text: "Save"
            }
        },
        State {
            name: "view"
            PropertyChanges {
                target: editButton
                text: "Edit"
            }
        }
    ]

    state: "view"

    Item {
        id: header

        width: parent.width
        height: 100

        Image {
            id: avatar
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: 10
            source: "dummydata/allisonreeves.jpg"
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
                text: "Allison Reeves"
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
                    source: "assets/icon_availability.png"
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
            text: "Edit"

            height: 30
            width: 100

            onClicked: {
                if (contactDetails.state == "view")
                    contactDetails.state = "edit"
                else
                    contactDetails.state = "view"
            }
        }
    }

    Component {
        id: sectionDelegate

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: 40

            Text {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 5
                text: section
            }
        }
    }

    Component {
        id: contactDetailsDelegate

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 1
            height: valueTextMulti.height > 30 ? valueTextMulti.height : 30

            Rectangle {
                id: contentBox
                border.color: "black"
                color: "white"

                anchors.left: parent.left
                anchors.right: actionBox.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                TextInput {
                    id: valueText
                    anchors.left: parent.left
                    anchors.right: typeText.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 5
                    text: value
                    readOnly: contactDetails.state == "view"
                    visible: (section !== "Address")
                }

                // Used to edit the address
                TextEdit {
                    id: valueTextMulti
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 5
                    height: paintedHeight
                    text: value
                    readOnly: contactDetails.state == "view"
                    visible: (section === "Address")
                }

                Text {
                    id: typeText
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: 5
                    text: type
                }

            }

            Rectangle {
                id: actionBox
                border.color: "black"
                color: "white"
                // TODO: add the icons
                width: 30
                height: parent.height
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right

                // TODO: replace with IconButton when it is done
                Image {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 5

                    source: actionIcon
                    fillMode: Image.PreserveAspectFit
                }
            }
        }
    }

    ListView {
        id: contactDetailsView

        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        section.delegate: sectionDelegate
        section.property: "section"
        clip: true

        delegate: contactDetailsDelegate

        model: ContactDetailsModel { }

    }
}
