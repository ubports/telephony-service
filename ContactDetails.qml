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

        delegate: ContactDetailsDelegate {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 1

            Connections {
                target: header
                onSaveClicked: {
                    contactDetailsItem.save();
                }
            }
        }

        model: contactdetails
    }
}
