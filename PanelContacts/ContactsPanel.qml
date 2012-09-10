import QtQuick 1.1
import TelephonyApp 0.1
import ".."
import "../Widgets"
import "../fontUtils.js" as Font
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

Item {
    id: contactsPanel

    property alias searchQuery : contactsSearchBox.searchQuery

    anchors.fill: parent
    signal contactClicked(variant contact)
    onContactClicked: telephony.showContactDetails(contact)

    SearchEntry {
        id: contactsSearchBox

        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10

        leftIconSource: text ? "../assets/quick_search_delete_icon.png" : "../assets/search_icon.png"
        hint: "Search contacts"
        onLeftIconClicked: text = ""
    }

    ListItem.Standard {
        id: newContact

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: contactsSearchBox.bottom

        control: PanelButton {
            text: "Add a new contact"
            iconSource: "../assets/add_contacts_icon.png"
            anchors.fill: parent
            onClicked: telephony.createNewContact()
        }

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
            selected: telephony.contactDetails.loaded && telephony.view.contact == contact
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
