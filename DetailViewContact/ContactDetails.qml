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
        contentHeight: detailsList.height + 32 + newDetailButton.height + 10

        Column {
            id: detailsList
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 1
            spacing: 16

            Repeater {
                model: (contact) ? [
                   { name: "Phone", delegateSource: "TextContactDetailsDelegate.qml",
                     items: contact.phoneNumbers, newItemText: "Add a phone number", newItemType: "PhoneNumber",
                     actionIcon: "../assets/icon_message_grey.png", displayField: "number" },
                   { name: "Email", delegateSource: "TextContactDetailsDelegate.qml",
                     items: contact.emails, newItemText: "Add an email address",  newItemType: "EmailAddress",
                     actionIcon: "../assets/icon_envelope_grey.png", displayField: "emailAddress" },
                   { name: "Address", delegateSource: "TextContactDetailsDelegate.qml",
                     items: contact.addresses, newItemText: "Add a postal address",  newItemType: "Address",
                     actionIcon: "../assets/icon_address.png", delegateSource: "AddressContactDetailsDelegate.qml" },
                   { name: "IM", delegateSource: "TextContactDetailsDelegate.qml",
                     items: contact.onlineAccounts, newItemText: "Add an online account",
                     displayField: "accountUri", newItemType: "OnlineAccount" }
                ] : []

                delegate: ContactDetailsSection {
                    anchors.left: (parent) ? parent.left : undefined
                    anchors.right: (parent) ? parent.right : undefined

                    detailTypeInfo: modelData
                    editable: contactDetails.editable

                    model: modelData.items
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
        }

        ColoredButton {
            id: newDetailButton
            color: "transparent"
            borderColor: "black"
            borderWidth: 1
            radius: 0
            height: newDetailButtonText.paintedHeight + newDetailButtonText.anchors.margins * 2

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: detailsList.bottom
            anchors.topMargin: 32
            anchors.leftMargin: 1
            anchors.rightMargin: 1

            TextCustom {
                id: newDetailButtonText
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right:  parent.right
                anchors.margins: 5
                fontSize: "x-large"
                text: "Add another field"
            }

            onClicked: console.log("New item")
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

                    console.log("Modified ?: " + contact.modified);

                    if (contact.modified) contact.save();
                    editable = false;
                }
            }
        }
    }
}
