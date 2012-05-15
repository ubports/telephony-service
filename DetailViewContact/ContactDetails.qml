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

    Flickable {
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        flickableDirection: Flickable.VerticalFlick
        clip: true

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 1

            // Phone section
            ContactDetailsSection {
                id: phoneSection
                editable: contactDetails.editable
                anchors.left: parent.left
                anchors.right: parent.right
                name: "Phone"
                model: (contact) ? contact.phoneNumbers : null
                delegate: ContactDetailsDelegate {
                    anchors.left: (parent) ? parent.left : undefined
                    anchors.right: (parent) ? parent.right : undefined
                    actionIcon: "../assets/icon_message_grey.png"
                    value: modelData.number
                    type: modelData.contexts.toString()

                    onClicked: telephony.startCallToContact(contact, value);
                    onActionClicked: telephony.startChat(contact);
                }
            }

            // Email section
            ContactDetailsSection {
                id: emailSection
                editable: contactDetails.editable
                anchors.left: parent.left
                anchors.right: parent.right
                name: "Email"
                model: (contact) ? contact.emails : null
                delegate: ContactDetailsDelegate {
                    anchors.left: (parent) ? parent.left : undefined
                    anchors.right: (parent) ? parent.right : undefined
                    actionIcon: "../assets/icon_envelope_grey.png"
                    value: modelData.emailAddress
                    type: "" // FIXME: there is no e-mail type it seems, but needs double checking in any case
                }
            }

            // IM section
            ContactDetailsSection {
                id: imSection
                editable: contactDetails.editable
                anchors.left: parent.left
                anchors.right: parent.right
                name: "IM"
                model: (contact) ? contact.onlineAccounts : null
                delegate: ContactDetailsDelegate {
                    anchors.left: (parent) ? parent.left : undefined
                    anchors.right: (parent) ? parent.right : undefined
                    actionIcon: "../assets/icon_chevron_right.png"
                    value: modelData.accountUri
                    type: modelData.serviceProvider
                }
            }

            // Address section
            ContactDetailsSection {
                id: addressSection
                editable: contactDetails.editable
                anchors.left: parent.left
                anchors.right: parent.right
                name: "Address"
                model: (contact) ? contact.addresses : null
                delegate: ContactDetailsDelegate {
                    anchors.left: (parent) ? parent.left : undefined
                    anchors.right: (parent) ? parent.right : undefined
                    actionIcon: "../assets/icon_chevron_right.png"
                    // FIXME: implement a better function to handle address formatting
                    value: modelData.street + "\n" + modelData.city + "\n" + modelData.state + "\n" + modelData.country
                    type: "" // FIXME: double check if QContact has an address type field
                    multiLine: true
                }
            } // ContactDetailsSection
        } // Column
    } // Flickable
}
