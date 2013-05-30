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
import ".."
import "../Widgets" as LocalWidgets
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

LocalWidgets.PhonePage {
    id: contactsPanel
    title: i18n.tr("Contacts")
    onActiveChanged: {
        // workaround to scroll ListView to the top.
        // We currently place the search bar inside the
        // ListView header, and it is not taken into
        // account when calculating the header offset.
        // Ref.: https://bugs.launchpad.net/phone-app/+bug/1181362
        if (active) {
            contactsList.contentY = -searchBar.height-header.height
        }
    }

    tools: ToolbarActions {
        Action {
            text: i18n.tr("Add")
            iconSource: Qt.resolvedUrl("../assets/new-contact.png")
            onTriggered: mainView.createNewContact()
        }
    }

    property alias searchQuery: contactProxyModel.filterText

    signal contactClicked(variant contact)
    onContactClicked: mainView.showContactDetails(contact, true)

    ContactProxyModel {
        id: contactProxyModel
        model: contactModel
        onCountChanged: {
            if (searchQuery != "") {
                contactsList.contentY = -searchBar.height-header.height
            }
        }
    }

    Component {
        id: contactDelegate
        ContactDelegate {
            onClicked: contactsPanel.contactClicked(model.contact)
            selected: (mainView.view &&
            mainView.view.contact &&
            typeof(contact) != "undefined") ? (mainView.view.contact == contact) : false
        }
    }

    Item {
        id: searchBar
        y: contactsPanel.header.y + contactsPanel.header.height + units.gu(1)
        x: contactsPanel.header.x
        height: contactsSearchBox.height + units.gu(1)
        anchors.left: parent.left
        anchors.right: parent.right
        z: 1

        TextField {
            id: contactsSearchBox
            objectName: "contactsSearchBox"

            anchors.left: parent.left
            anchors.leftMargin: units.gu(1)
            anchors.right: parent.right
            anchors.rightMargin: units.gu(1)
            height: units.gu(4)
            z: 2

            //placeholderText: i18n.tr("Search")
            Keys.onEscapePressed: text = ""
            onTextChanged: {
                contactsPanel.header.y = 0
            }

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
    }


    ListView {
        id: contactsList
        objectName: "contactsList"

        anchors.top: parent.top
        anchors.topMargin: searchBar.height
        anchors.bottom: keyboard.top
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true
        model: contactProxyModel
        cacheBuffer: height * 2
        currentIndex: -1
        z: 0

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
    }

    LocalWidgets.KeyboardRectangle {
        id: keyboard
    }
}
