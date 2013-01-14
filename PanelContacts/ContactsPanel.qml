/*
 * Copyright 2012 Canonical Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import TelephonyApp 0.1
import ".."
import "../Widgets" as LocalWidgets
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

LocalWidgets.TelephonyPage {
    id: contactsPanel
    title: "Contacts"

    chromeButtons: ListModel {
        ListElement {
            label: "Add"
            name: "add"
            icon: "../assets/new-contact.png"
        }
    }

    onChromeButtonClicked: {
        if (buttonName == "add") {
            telephony.createNewContact()
        }
    }

    property alias searchQuery: contactProxyModel.filterText

    anchors.fill: parent
    signal contactClicked(variant contact)
    onContactClicked: telephony.showContactDetails(contact, true)

    ContactProxyModel {
        id: contactProxyModel
        model: contactModel
    }

    Component {
        id: contactDelegate
        ContactDelegate {
            onClicked: contactsPanel.contactClicked(model.contact)
            selected: (telephony.view &&
            telephony.view.contact &&
            typeof(contact) != "undefined") ? (telephony.view.contact == contact) : false
        }
    }

    ListView {
        id: contactsList
        objectName: "contactsList"

        anchors.top: parent.top
        anchors.topMargin: units.gu(1)
        anchors.bottom: keyboard.top
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true
        model: contactProxyModel

        header: Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: units.gu(1)

            TextField {
                id: contactsSearchBox
                objectName: "contactsSearchBox"

                anchors.left: parent.left
                anchors.leftMargin: units.gu(1)
                anchors.right: parent.right
                anchors.rightMargin: units.gu(1)
                height: units.gu(4)

                //placeholderText: "Search"
                Keys.onEscapePressed: text = ""

                primaryItem: AbstractButton {
                    width: units.gu(3)
                    Image {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: units.gu(0.5)
                        source: "../assets/search.png"
                    }
                    onClicked: contactsSearchBox.text = ""
                }

                Binding {
                    target: contactsPanel
                    property: "searchQuery"
                    value: contactsSearchBox.text
                }
            }

            ListItem.ThinDivider {
                visible: !telephony.singlePane
            }

            ListItem.Standard {
                id: newContact

                anchors.left: parent.left
                anchors.right: parent.right
                height: visible ? units.gu(4) : 0
                visible: !telephony.singlePane
                __leftIconMargin: units.gu(3)
                __rightIconMargin: units.gu(2)

                text: "Add a new contact"
                icon: Qt.resolvedUrl("../assets/add_contacts_icon.png")
                iconFrame: false
                onClicked: telephony.createNewContact()

                selected: telephony.contactDetails.loaded && telephony.view.added
            }
        }

        delegate: Loader {
            id: contactLoader
            sourceComponent: contactDelegate
            asynchronous: true
            height: item ? item.height : units.gu(8)
            anchors.left: parent.left
            anchors.right: parent.right

            Binding {
                target: contactLoader.item
                property: "model"
                value: model
                when: contactLoader.status == Loader.Ready
            }
        }
        section.property: "initial"
        section.criteria: ViewSection.FullString
        section.delegate: LocalWidgets.ListSectionHeader {
            width: parent ? parent.width : 0
            text: typeof(section) != "undefined" ? section : ""
        }
    }

    Scrollbar {
        flickableItem: contactsList
        align: Qt.AlignTrailing
        __interactive: false
    }

    LocalWidgets.KeyboardRectangle {
        id: keyboard
    }
}
