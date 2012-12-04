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

    property alias searchQuery : contactsSearchBox.text

    anchors.fill: parent
    signal contactClicked(variant contact)
    onContactClicked: telephony.showContactDetails(contact, true)

   LocalWidgets.Header {
        id: header
        text: title
    }

    TextField {
        id: contactsSearchBox

        anchors.top: header.bottom
        anchors.topMargin: units.gu(1)
        anchors.left: parent.left
        anchors.leftMargin: units.gu(1)
        anchors.right: parent.right
        anchors.rightMargin: units.gu(1)

        placeholderText: "Search contacts"
        Keys.onEscapePressed: text = ""

        primaryItem: AbstractButton {
            width: units.gu(3)
            Image {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: units.gu(0.5)
                source: contactsSearchBox.text ? "../assets/quick_search_delete_icon.png" : "../assets/search_icon.png"
            }
            onClicked: contactsSearchBox.text = ""
        }
    }

    Column {
        id: buttons
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: contactsSearchBox.bottom
        anchors.topMargin: units.gu(1)

        // hide this on single panel version
        visible: !telephony.singlePane
        height: visible ? childrenRect.height : 0

        ListItem.ThinDivider {}

        ListItem.Standard {
            id: newContact

            anchors.left: parent.left
            anchors.right: parent.right
            height: units.gu(4)
            __leftIconMargin: units.gu(3)
            __rightIconMargin: units.gu(2)

            text: "Add a new contact"
            icon: Qt.resolvedUrl("../assets/add_contacts_icon.png")
            iconFrame: false
            onClicked: telephony.createNewContact()

            selected: telephony.contactDetails.loaded && telephony.view.added
        }
    }

    ContactProxyModel {
        id: contactProxyModel
        filterText: contactsSearchBox.text
        model: contactModel
    }

    ContactProxyModel {
        id: favoriteContactProxyModel
        filterText: contactsSearchBox.text
        favoriteOnly: true
        model: contactModel
    }

    Component {
        id: contactDelegate
        ContactDelegate {
            onClicked: contactsPanel.contactClicked(contact)
            selected: (telephony.view &&
            telephony.view.contact &&
            typeof(contact) != "undefined") ? (telephony.view.contact == contact) : false
        }
    }

    ListView {
        id: contactsList
        objectName: "contactsList"

        anchors.top: buttons.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true
        // FIXME: references to runtime and fake model need to be removed before final release
        model: typeof(runtime) != "undefined" ? fakeContacts : contactProxyModel

        header: Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: headerFavorite.height + favoriteContacts.height
            visible: favoriteContacts.count > 0

            LocalWidgets.ListSectionHeader {
                id: headerFavorite
                anchors.left: parent.left
                anchors.right: parent.right
                text: "Favorite"
            }
            
           ListView {
                id: favoriteContacts
                model: typeof(runtime) != "undefined" ? fakeContacts : favoriteContactProxyModel
                clip: true
                interactive: false
                anchors.top: headerFavorite.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: contentHeight
                delegate: contactDelegate
            }
        }
        delegate: contactDelegate
        section.property: "initial"
        section.criteria: ViewSection.FullString
        section.delegate: LocalWidgets.ListSectionHeader {
            width: parent ? parent.width : 0
            text: typeof(section) != "undefined" ? section : ""
        }
    }

    LocalWidgets.ScrollbarForListView {
        view: contactsList
    }
}
