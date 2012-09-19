import QtQuick 1.1
import TelephonyApp 0.1
import ".."
import "../Widgets"
import "../fontUtils.js" as Font
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

Item {
    id: contactsPanel

    property alias searchQuery : contactsSearchBox.text

    anchors.fill: parent
    signal contactClicked(variant contact)
    onContactClicked: telephony.showContactDetails(contact, true)

    TextField {
        id: contactsSearchBox

        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10

        placeholderText: "Search contacts"
        Keys.onEscapePressed: text = ""

        primaryItem: AbstractButton {
            width: 20
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            Image {
                anchors.centerIn: parent
                source: contactsSearchBox.text ? "../assets/quick_search_delete_icon.png" : "../assets/search_icon.png"
            }
            onClicked: contactsSearchBox.text = ""
        }
    }

    ListItem.Standard {
        id: newContact

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: contactsSearchBox.bottom
        anchors.topMargin: 10
        height: 30

        text: "Add a new contact"
        iconSource: "../assets/add_contacts_icon.png"
        iconFrame: false
        onClicked: telephony.createNewContact()

//        selected: telephony.contactDetails.loaded && telephony.view.added
    }

    ContactProxyModel {
        id: contactProxyModel
        filterText: contactsSearchBox.text
        model: contactModel
    }

    ListView {
        id: contactsList
        anchors.top: newContact.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true
        // FIXME: references to runtime and fake model need to be removed before final release
        model: typeof(runtime) != "undefined" ? fakeContacts : contactProxyModel

        delegate: ContactDelegate {
            onClicked: contactsPanel.contactClicked(contact)
            selected: telephony.view.contact && telephony.view.contact == contact
        }

        section.property: "initial"
        section.criteria: ViewSection.FullString
        section.delegate: ListSectionHeader {
            width: parent.width
            text: section
        }
    }

    ScrollbarForListView {
        view: contactsList
    }
}
