/*
 * Copyright 2012-2013 Canonical Ltd.
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Ubuntu.PhoneApp 0.1
import Ubuntu.Components 0.1
import "../Widgets" as LocalWidgets
import "../"
import "DetailTypeUtilities.js" as DetailTypes

LocalWidgets.PhonePage {
    id: contactDetails

    property bool editable: false
    property alias contact: contactWatcher.contact
    property variant contactId: (contact) ? contact.id : null
    property bool added: false

    ContactWatcher {
        id: contactWatcher
    }

    onContactChanged: editable = false

    title: "Contact Details"

    tools: ToolbarActions {

        Action {
            text: "Edit"
            iconSource: Qt.resolvedUrl("../assets/edit.png")
            visible: !editable
            enabled: visible
            onTriggered: editable = true
        }

        Action {
            text: "Delete"
            iconSource: Qt.resolvedUrl("../assets/delete.png")
            visible: editable && !added
            enabled: visible
            onTriggered: {
                // FIXME: show a dialog asking for confirmation
                contactModel.removeContact(contact);
                mainView.resetView();
            }
        }

        Action {
            text: "Cancel"
            iconSource: Qt.resolvedUrl("../assets/cancel.png")
            visible: editable
            enabled: visible
            onTriggered: {
                if (added) {
                    mainView.resetView();
                } else {
                    contact.revertChanges();
                    editable = false;
                }
            }
        }

        Action {
            text: "Save"
            iconSource: Qt.resolvedUrl("../assets/save.png")
            visible: editable
            enabled: visible
            onTriggered: contactDetails.save();
        }
    }

    headerContents: ContactDetailsHeader {
        id: header
        contact: contactDetails.contact
        editable: contactDetails.editable
        focus: true
        backgroundColor: "#ededed"
    }

    function createNewContact() {
        contact = Qt.createQmlObject("import Ubuntu.PhoneApp 0.1; ContactEntry {}", contactModel);
        editable = true;
        added = true;

        for (var i = 0; i < detailsList.children.length; i++) {
            var child = detailsList.children[i];
            if (child.detailTypeInfo && child.detailTypeInfo.createOnNew) {
                child.appendNewItem();
            }
        }

        header.focus = true;
    }

    function save() {
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
        added = false;
    }

    Connections {
        target: contactModel
        onContactSaved: {
            // once the contact gets saved after editing, we reload it in the view
            // because for added contacts, we need the newly created ContactEntry instead of the one
            // we were using before.
            contactWatcher.contact = null;
            // empty contactId because if it remains same, contact watcher wont search
            // for a new contact
            contactWatcher.contactId = ""
            contactWatcher.contactId = contactId;
        }

        onContactRemoved: {
            if (contactId == contactDetails.contactId) {
                contactDetails.contact = null;
                mainView.resetView();
            }
        }
    }

    Flickable {
        id: scrollArea

        anchors.top: parent.top
        anchors.bottom: editFooter.top
        anchors.left: parent.left
        anchors.right: parent.right
        flickableDirection: Flickable.VerticalFlick
        clip: true
        contentHeight: detailsList.height

        Behavior on contentY {
            LocalWidgets.StandardAnimation { }
        }

        Column {
            id: detailsList

            anchors.left: parent.left
            anchors.right: parent.right

            function scrollToSectionPosition(section, y, height) {
                var position = scrollArea.contentItem.mapFromItem(section, 0, y);

                // check if the item is already visible
                var bottomY = scrollArea.contentY + scrollArea.height
                var itemBottom = position.y + height
                if (position.y >= scrollArea.contentY && itemBottom <= bottomY) {
                    return;
                }

                // if it is not, try to scroll and make it visible
                var targetY = position.y + height - scrollArea.height
                if (targetY >= 0 && position.y) {
                    scrollArea.contentY = targetY;
                } else if (position.y < scrollArea.contentY) {
                    // if it is hidden at the top, also show it
                    scrollArea.contentY = position.y;
                }
            }

            Repeater {
                model: (contact) ? DetailTypes.supportedTypes : []

                delegate: ContactDetailsSection {
                    id: section
                    anchors.left: (parent) ? parent.left : undefined
                    anchors.right: (parent) ? parent.right : undefined

                    detailTypeInfo: modelData
                    editable: contactDetails.editable
                    onDetailAdded: focus = true

                    onScrollRequested: detailsList.scrollToSectionPosition(section, y, height)

                    model: (contact) ? contact[modelData.items] : []
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
                            onActionClicked: {
                                switch(modelData.type) {
                                case ContactDetail.PhoneNumber:
                                    var filterProperty = "contactId";
                                    var filterValue = contact.id;
                                    var phoneNumber = modelData.number;
                                    mainView.showCommunication(filterProperty, filterValue, phoneNumber, contact.id, true);
                                    break;
                                case ContactDetail.EmailAddress:
                                    Qt.openUrlExternally("mailto:" + modelData.emailAddress);
                                    break;
                                default:
                                    break;
                                }
                            }
                            onClicked: {
                                switch (modelData.type) {
                                case ContactDetail.PhoneNumber:
                                    mainView.callNumber(modelData.number);
                                    break;
                                case ContactDetail.EmailAddress:
                                    Qt.openUrlExternally("mailto:" + modelData.emailAddress);
                                    break;
                                }
                            }
                            onScrollRequested: {
                                var position = section.mapFromItem(item, item.x, item.y)
                                section.scrollRequested(position.y, item.height)
                            }
                        }
                    }
                }
            }
        }
    }

    Scrollbar {
        flickableItem: scrollArea
        align: Qt.AlignTrailing
        __interactive: false
    }

    Item {
        id: editFooter

        anchors.bottom: keyboard.top
        anchors.left: parent.left
        anchors.right: parent.right

        visible: !mainView.singlePane
        height: visible ? units.gu(5) : 0

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
            height: units.dp(1)
            color: "white"
        }

        Item {
            id: footerButtons

            anchors.top: separator.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            Button {
                id: deleteButton

                height: units.gu(3)
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: units.gu(1)
                text: "Delete"
                opacity: (editable && !added) ? 1.0 : 0.0

                onClicked: {
                    // FIXME: show a dialog asking for confirmation
                    contactModel.removeContact(contact);
                    mainView.resetView();
                }
            }

            Button {
                id: cancelButton

                height: units.gu(3)
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: editSaveButton.left
                anchors.rightMargin: units.gu(1)
                text: "Cancel"
                opacity: (editable) ? 1.0 : 0.0
                onClicked: {
                    if (added) {
                        mainView.resetView();
                    } else {
                        contact.revertChanges();
                        editable = false;
                    }
                }
            }

            Button {
                id: editSaveButton
                objectName: "editSaveButton"

                height: units.gu(3)
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: units.gu(1)
                color: (editable) ? "#dd4f22" : "#e3e5e8"
                text: (editable) ? "Save" : "Edit"
                enabled: !editable || header.contactNameValid
                onClicked: {
                    if (!editable) editable = true;
                    else {
                        contactDetails.save();
                    }
                }
            }
        }
    }

    LocalWidgets.KeyboardRectangle {
        id: keyboard
    }
}
