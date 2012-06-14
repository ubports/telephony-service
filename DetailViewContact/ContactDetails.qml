import QtQuick 1.1
import QtMobility.contacts 1.1
import TelephonyApp 0.1
import "../Widgets"
import "DetailTypeUtilities.js" as DetailTypes
// FIXME: write a different delegate for the call log shown in the contact
// details once we get wireframes or visual designs for that
import "../DetailViewCallLog"

Item {
    id: contactDetails

    property string viewName: "contacts"
    property bool editable: false
    property variant contact: null
    property variant added: false

    onContactChanged: editable = false

    width: 400
    height: 600

    function createNewContact(contactsModel) {
        contact = Qt.createQmlObject("import QtMobility.contacts 1.1; Contact {}", contactsModel);
        editable = true;
        added = true;
    }

    ContactDetailsHeader {
        id: header
        contact: contactDetails.contact
        editable: contactDetails.editable
    }

    Flickable {
        id: scrollArea
        anchors.top: header.bottom
        anchors.bottom: editFooter.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        flickableDirection: Flickable.VerticalFlick
        clip: true
        contentHeight: detailsList.height + 32 + newDetailChooser.height + 10

        Column {
            id: detailsList
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 1
            spacing: 16

            Repeater {
                model: (contact) ? DetailTypes.supportedTypes : []

                delegate: ContactDetailsSection {
                    anchors.left: (parent) ? parent.left : undefined
                    anchors.right: (parent) ? parent.right : undefined

                    detailTypeInfo: modelData
                    editable: contactDetails.editable

                    model: contact[modelData.items]
                    delegate: Loader {
                        anchors.left: (parent) ? parent.left : undefined
                        anchors.right: (parent) ? parent.right : undefined

                        source: detailTypeInfo.delegateSource

                        Binding { target: item; property: "detail"; value: modelData }
                        Binding { target: item; property: "detailTypeInfo"; value: detailTypeInfo }
                        Binding { target: item; property: "editable"; value: contactDetails.editable }

                        Connections {
                            target: item
                            ignoreUnknownSignals: true

                            onDeleteClicked: contact.removeDetail(modelData)
                            onActionClicked: if (modelData.type == ContactDetail.PhoneNumber) telephony.startChat(contact, modelData.number);
                            onClicked: if (modelData.type == ContactDetail.PhoneNumber) telephony.startCallToContact(contact, modelData.number);
                        }
                    }
                }
            }


            // Call Log section
            ContactDetailsSection {
                id: callLogSection
                editable: false
                anchors.left: parent.left
                anchors.right: parent.right

                detailTypeInfo: { return { name: "Call Log" } }

                model: CallLogProxyModel {
                    logModel: callLogModel
                    contactId: (contact) ? contact.guid.guid : "some string that won't match"
                }

                delegate: CallLogDelegate {
                    id: delegate
                    anchors.left: (parent) ? parent.left : undefined
                    anchors.right: (parent) ? parent.right : undefined

                    onClicked: telephony.showContactDetailsFromId(contactId)
                    onActionClicked: telephony.callNumber(phoneNumber)
                }
            }
        }

        ContactDetailTypeChooser {
            id: newDetailChooser

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: detailsList.bottom
            anchors.topMargin: 32
            anchors.leftMargin: 1
            anchors.rightMargin: 1

            opacity: (editable) ? 1.0 : 0.0
            contact: (editable) ? contactDetails.contact : null

            onSelected: {
                for (var i = 0; i < detailsList.children.length; i++) {
                    var child = detailsList.children[i];
                    if (child.detailTypeInfo.name == detailType.name) {
                        child.appendNewItem();
                        return;
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
                    var addedDetails = [];
                    for (var i = 0; i < detailsList.children.length; i++) {
                        var saver = detailsList.children[i].save;
                        if (saver && saver instanceof Function) {
                            var newDetails = saver();
                            for (var k = 0; k < newDetails.length; k++)
                                addedDetails.push(newDetails[k]);
                        }
                    }

                    for (i = 0; i < addedDetails.length; i++) {
                        console.log("Add detail: " + contact.addDetail(addedDetails[i]));
                    }

                    console.log("Modified ?: " + contact.modified + " id " + contact.contactId);

                    if (contactDetails.added) contact.parent.saveContact(contact);
                    console.log("Modified ?: " + contact.modified + " id " + contact.contactId);
                    if (contact.modified) contact.save();
                    console.log("Modified ?: " + contact.modified + " id " + contact.contactId);
                    editable = false;
                }
            }
        }
    }
}
