import QtQuick 1.1

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

            // TODO: use TextCustom
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
            id: contactDetailsItem
            anchors.left: (parent) ? parent.left : undefined
            anchors.right: (parent) ? parent.right : undefined
            anchors.margins: 1
            editable: contactDetails.editable

            onPhoneClicked: {
                telephony.startCallToContact(contactDetails.contact)
            }

            onMessageClicked: {
                telephony.startChat(contactDetails.contact)
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
