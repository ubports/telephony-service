// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    id: contactDetails

    property bool editable: false
    property variant contact: null

    width: 400
    height: 600

    Component.onCompleted: {
        contact = contacts.get(3)
    }

    ContactDetailsHeader {
        id: header
        contact: contactDetails.contact

        onEditClicked: {
            contactDetails.editable = true
        }

        onSaveClicked: {
            contactDetails.editable = false
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
            id: contactDetailsItem

            property bool editable: false
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 1
            height: childrenRect.height

            Connections {
                target: header
                onSaveClicked: {
                    contactDetailsItem.save();
                }
            }

            function save() {
                var newValue

                if (section == "Address") {
                    newValue = valueTextMulti.text
                } else {
                    newValue = valueText.text
                }

                contactdetails.set(index, { "value": newValue })
            }

            Rectangle {
                id: contentBox
                border.color: "black"
                color: "white"

                anchors.left: parent.left
                anchors.right: actionBox.left
                anchors.top: parent.top
                height: valueTextMulti.height > 30 ? valueTextMulti.height : 30

                TextInput {
                    id: valueText
                    anchors.left: parent.left
                    anchors.right: typeText.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 5
                    text: value
                    readOnly: !contactDetails.editable
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
                    readOnly: !contactDetails.editable
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

        model: contactdetails
    }
}
