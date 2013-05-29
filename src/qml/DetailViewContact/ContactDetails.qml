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

    title: i18n.tr("Contact Details")

    tools: ToolbarActions {
        lock: editable

        Action {
            text: i18n.tr("Edit")
            iconSource: Qt.resolvedUrl("../assets/edit.png")
            visible: !editable
            enabled: visible
            onTriggered: editable = true
        }

        Action {
            id: deleteAction

            text: i18n.tr("Delete")
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
            id: cancelAction

            text: i18n.tr("Cancel")
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
            id: saveAction

            text: i18n.tr("Save")
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
        onFavoriteSaved: contactDetails.save()
    }

    function createNewContact(number) {
        var newContact = Qt.createQmlObject("import Ubuntu.PhoneApp 0.1; ContactEntry {}", contactModel);
        if (number != null) {
            var phoneNumber = Qt.createQmlObject("import Ubuntu.PhoneApp 0.1; ContactPhoneNumber { number: \"" + number + "\"; }", contactModel);
            newContact.addDetail(phoneNumber);
        }
        contact = newContact;
        editable = true;
        added = true;

        for (var i = 0; i < detailsList.children.length; i++) {
            var child = detailsList.children[i];
            if (child.detailTypeInfo && child.detailTypeInfo.createOnNew) {
                // if we already added a phone number, do not add another empty one
                if (child.detailTypeInfo.newItemType == "ContactPhoneNumber" && number != null) {
                    continue;
                }
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
        anchors.bottom: editToolbar.top
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

    EditToolbar {
        id: editToolbar
        visible: editable
        anchors.bottom: keyboard.top

        onDeleteClicked: deleteAction.triggered(editToolbar)
        onCancelClicked: cancelAction.triggered(editToolbar)
        onSaveClicked: saveAction.triggered(editToolbar)

        onVisibleChanged: {
            if (visible) {
                // hide the toolbar
                toolbar.opened = false;
            }
        }
    }

    Scrollbar {
        flickableItem: scrollArea
        align: Qt.AlignTrailing
    }

    LocalWidgets.KeyboardRectangle {
        id: keyboard
    }
}
