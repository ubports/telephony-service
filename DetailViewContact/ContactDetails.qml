import QtQuick 1.1
import "../Widgets"

Item {
    id: contactDetails

    property string viewName: "contacts"
    property bool editable: false
    property variant contact: null

    width: 400
    height: 600

    onEditableChanged: {
        // If editable has just changed to false it means we just finished
        // editing and it's time to save (if anything was changed)
        if (!editable && contact.modified) {
            contact.save()
        }
    }

    ContactDetailsHeader {
        id: header
        contact: contactDetails.contact
        editable: contactDetails.editable

        onEditClicked: contactDetails.editable = !contactDetails.editable
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
                    editable: contactDetails.editable

                    onClicked: telephony.startCallToContact(contact, value);
                    onActionClicked: telephony.startChat(contact, number);
                    onFieldValueChanged: modelData.number = newValue;
                }
            }

            // Email section
            ContactDetailsSection {
                id: emailSection
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
                    editable: contactDetails.editable
                    onFieldValueChanged: modelData.emailAddress = newValue;
                }
            }

            // IM section
            ContactDetailsSection {
                id: imSection
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
                    editable: contactDetails.editable
                    onFieldValueChanged: modelData.accountUri = newValue;
                }
            }


            // Address section
            ContactDetailsSection {
                id: addressSection
                anchors.left: parent.left
                anchors.right: parent.right
                name: "Address"
                model: (contact) ? contact.addresses : null
                delegate: ContactDetailsDelegate {
                    anchors.left: (parent) ? parent.left : undefined
                    anchors.right: (parent) ? parent.right : undefined
                    actionIcon: "../assets/icon_chevron_right.png"

                    // Format in the same way as Android's default address book
                    function nonEmpty(item) { return item && item.length > 0 }
                    value: [
                      modelData.street,
                      [ [modelData.locality, modelData.region].filter(nonEmpty).join(", "),
                        modelData.postcode
                      ].filter(nonEmpty).join(" "),
                      modelData.country
                    ].filter(nonEmpty).join("\n");

                    type: "" // FIXME: double check if QContact has an address type field
                    multiLine: true
                    editable: contactDetails.editable
                    // TODO: this has probably to be edited as multiple fields
                }
            } // ContactDetailsSection
        } // Column
    } // Flickable
}
