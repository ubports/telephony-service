import QtQuick 1.1
import "../Widgets"

Item {
    id: contactDetails

    property string viewName: "contacts"
    property bool editable: false
    property variant contact: null

    width: 400
    height: 600

    ContactDetailsHeader {
        id: header
        contact: contactDetails.contact
        editable: contactDetails.editable
    }

    Flickable {
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.bottomMargin: editFooter.height
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        flickableDirection: Flickable.VerticalFlick
        clip: true

        Column {
            id: detailsList
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
                delegate: TextContactDetailsDelegate {
                    anchors.left: (parent) ? parent.left : undefined
                    anchors.right: (parent) ? parent.right : undefined
                    actionIcon: "../assets/icon_message_grey.png"
                    type: modelData.contexts.toString()
                    editable: contactDetails.editable
                    contactModelItem: modelData
                    contactModelProperty: "number"

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
                delegate: TextContactDetailsDelegate {
                    anchors.left: (parent) ? parent.left : undefined
                    anchors.right: (parent) ? parent.right : undefined
                    actionIcon: "../assets/icon_envelope_grey.png"
                    contactModelItem: modelData
                    contactModelProperty: "emailAddress"

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
                delegate: TextContactDetailsDelegate {
                    anchors.left: (parent) ? parent.left : undefined
                    anchors.right: (parent) ? parent.right : undefined
                    actionIcon: "../assets/icon_chevron_right.png"
                    type: modelData.serviceProvider
                    contactModelItem: modelData
                    contactModelProperty: "accountUri"
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
                delegate: AddressContactDetailsDelegate {
                    anchors.left: (parent) ? parent.left : undefined
                    anchors.right: (parent) ? parent.right : undefined
                    actionIcon: "../assets/icon_address.png"

                    contactModelItem: modelData

                    type: "" // FIXME: double check if QContact has an address type field
                    editable: contactDetails.editable
                }
            } // ContactDetailsSection
        } // Column
    } // Flickable

    Rectangle {
        id: editFooter
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 50
        color: "grey"

        TextButton {
            id: deleteButton
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 10
            text: "Delete"
            color: "white"
            radius: 5
            height: 30
            width: 70
            opacity: (editable) ? 1.0 : 0.0
        }

        TextButton {
            id: cancelButton
            anchors.top: parent.top
            anchors.right: editSaveButton.left
            anchors.margins: 10
            text: "Cancel"
            color: "white"
            radius: 5
            height: 30
            width: 70
            opacity: (editable) ? 1.0 : 0.0
            onClicked: editable = false
       }

        TextButton {
            id: editSaveButton
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 10
            text: (editable) ? "Save" : "Edit"
            color: "white"
            radius: 5
            height: 30
            width: 70
            onClicked: {
                if (!editable) editable = true;
                else {
                    /* We ask each detail delegate to save all edits to the underlying
                       model object. The other way to do it would be to change editable
                       to false and catch onEditableChanged in the delegates and save there.
                       However that other way doesn't work since we can't guarantee that all
                       delegates have received the signal before we call contact.save() here.
                    */
                    for (var i = 0; i < detailsList.children.length; i++) {
                        var saver = detailsList.children[i].save;
                        if (saver && saver instanceof Function) saver();
                    }

                    console.log("Modified ?: " + contact.modified);
                    console.log("Phone numbers:");
                    for (i = 0; i < contact.phoneNumbers.length; i++) {
                        console.log(contact.phoneNumbers[i].number);
                    }

                    if (contact.modified) contact.save();
                    editable = false;
                }
            }
        }
    }
}
