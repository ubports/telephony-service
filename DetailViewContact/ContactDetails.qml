import QtQuick 1.1
import "../Widgets"

Item {
    id: contactDetails

    property bool editable: false
    property variant contact: null

    width: 400
    height: 600

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

            TextCustom {
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
            id: contactDetailsItem
            anchors.left: (parent) ? parent.left : undefined
            anchors.right: (parent) ? parent.right : undefined
            anchors.margins: 1
            editable: contactDetails.editable

            onClicked: {
                // we only handle clicks in the phone number for now
                switch (section) {
                case "Phone":
                    telephony.startCallToContact(contactDetails.contact)
                    break;
                default:
                    break;
                }
            }

            onActionClicked: {
                switch (section) {
                case "Phone":
                    telephony.startChat(contactDetails.contact)
                    break;
                default:
                    break;
                }
            }

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
