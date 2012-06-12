import QtQuick 1.1
import QtMobility.contacts 1.1
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
        anchors.bottom: editFooter.top
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
            spacing: 16

            Repeater {
                id: detailsCreator
                model: (contact) ? [
                   { name: "Phone",
                     items: contact.phoneNumbers, addText: "Add a phone number",
                     actionIcon: "../assets/icon_message_grey.png", displayField: "number" },
                   { name: "Email",
                     items: contact.emails, addText: "Add an email address",
                     actionIcon: "../assets/icon_envelope_grey.png", displayField: "emailAddress" },
                   { name: "Address",
                     items: contact.addresses, addText: "Add a postal address",
                     actionIcon: "../assets/icon_address.png" },
                   { name: "IM",
                     items: contact.onlineAccounts, addText: "Add an online account",
                     displayField: "accountUri" }
                ] : []

                delegate: ContactDetailsSection {
                    anchors.left: (parent) ? parent.left : undefined
                    anchors.right: (parent) ? parent.right : undefined

                    name: modelData.name
                    addText: modelData.addText
                    editable: contactDetails.editable

                    property variant parentModel: modelData
                    model: modelData.items
                    delegate: Loader {
                        anchors.left: (parent) ? parent.left : undefined
                        anchors.right: (parent) ? parent.right : undefined

                        source: {
                            // For now the address is the only type that doesn't use the generic text delegate
                            if (modelData.type == ContactDetail.Address) return "AddressContactDetailsDelegate.qml";
                            else return "TextContactDetailsDelegate.qml"
                        }

                        Binding { target: item; property: "detail"; value: modelData }
                        Binding { target: item; property: "type"; value: modelData.contexts.toString() }
                        Binding { target: item; property: "editable"; value: contactDetails.editable }

                        Binding { target: item; property: "contactModelProperty"; value: parentModel.displayField }
                        Binding { target: item; property: "actionIcon"; value: parentModel.actionIcon }

                        Connections {
                            target: item
                            ignoreUnknownSignals: true

                            onDeleteClicked: contact.removeDetail(detail)
                            onActionClicked: if (modelData.type == ContactDetail.PhoneNumber) telephony.startChat(contact, number);
                            onClicked: if (modelData.type == ContactDetail.PhoneNumber) telephony.startCallToContact(contact, value);
                        }
                    }
                }
            }
        }
    }

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

                    //if (contact.modified) contact.save();
                    editable = false;
                }
            }
        }
    }
}
