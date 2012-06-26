import QtQuick 1.1
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

    function createNewContact() {
        contact = Qt.createQmlObject("import TelephonyApp 0.1; ContactEntry {}", contactModel);
        editable = true;
        added = true;
    }

    Item {
        id: background

        anchors.fill: parent

        Image {
            anchors.fill: parent
            source: "../assets/noise_tile.png"
            fillMode: Image.Tile
        }

        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.05
        }
    }

    Rectangle {
        anchors.top: header.bottom
        anchors.bottom: editFooter.top
        anchors.left: parent.left
        anchors.right: parent.right
        color: "white"
        opacity: 0.3
    }

    Rectangle {
        anchors.fill: header
        color: "white"
        opacity: 0.5
    }

    ContactDetailsHeader {
        id: header
        contact: contactDetails.contact
        editable: contactDetails.editable
    }

    Image {
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        source: "../assets/right_pane_pattern.png"
        fillMode: Image.Tile
    }

    Flickable {
        id: scrollArea

        anchors.top: header.bottom
        anchors.bottom: editFooter.top
        anchors.left: parent.left
        anchors.right: parent.right
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        contentHeight: detailsList.height + (contactDetails.editable ? 32 + newDetailChooser.height + 10 : 0)

        Column {
            id: detailsList

            anchors.left: parent.left
            anchors.right: parent.right

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
                            onClicked: if (modelData.type == ContactDetail.PhoneNumber) telephony.callNumber(modelData.number);
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
                opacity: (contactDetails.editable) ? 0.0 : 1.0

                detailTypeInfo: { return { name: "Call Log" } }

                CallLogProxyModel {
                    id: proxyModel
                    logModel: callLogModel
                    contactId: (contact) ? contact.id : "some string that won't match"
                }
                // FIXME: references to runtime and fake model need to be removed before final release
                model: typeof(runtime) != "undefined" ? fakeCallLog : proxyModel

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

    Item {
        id: editFooter

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        height: 36

        Rectangle {
            anchors.fill: parent
            color: "white"
            opacity: 0.5
        }

        Rectangle {
            id: separator

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: "white"
        }

        Item {
            id: footerButtons

            anchors.top: separator.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            ButtonSmall {
                id: deleteButton

                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 10
                text: "Delete"
                opacity: (editable) ? 1.0 : 0.0
            }

            ButtonSmall {
                id: cancelButton

                anchors.verticalCenter: parent.verticalCenter
                anchors.right: editSaveButton.left
                anchors.rightMargin: 10
                text: "Cancel"
                opacity: (editable) ? 1.0 : 0.0
                onClicked: {
                    if (added) {
                        telephony.resetView();
                    } else {
                        editable = false;
                    }
                }
           }

            ButtonSmall {
                id: editSaveButton

                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 10
                color: editable ? "#dd4f22" : "#e3e5e8"
                text: (editable) ? "Save" : "Edit"
                onClicked: {
                    if (!editable) editable = true;
                    else {
                        /* We ask each detail delegate to save all edits to the underlying
                           model object. The other way to do it would be to change editable
                           to false and catch onEditableChanged in the delegates and save there.
                           However that other way doesn't work since we can't guarantee that all
                           delegates have received the signal before we call contact.save() here.
                        */
                        header.save();

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

                        if (contact.modified || added)
                            contactModel.saveContact(contact);

                        editable = false;
                    }
                }
            }
        }
    }
}
