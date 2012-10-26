import QtQuick 2.0
import TelephonyApp 0.1
import ".."
import "../Widgets" as LocalWidgets
import "../fontUtils.js" as Font
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

LocalWidgets.TelephonyPage {
    id: contactsPanel
    title: "Contacts"

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
            Image {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 5
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
        anchors.topMargin: 10

        ListItem.ThinDivider {}

        ListItem.Standard {
            id: newContact

            anchors.left: parent.left
            anchors.right: parent.right
            __height: 30
            __leftIconMargin: 20
            __rightIconMargin: 17

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

    ListView {
        id: contactsList
        anchors.top: buttons.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true
        // FIXME: references to runtime and fake model need to be removed before final release
        model: typeof(runtime) != "undefined" ? fakeContacts : contactProxyModel

        delegate: ContactDelegate {
            onClicked: contactsPanel.contactClicked(contact)
            selected: (telephony.view && 
                       telephony.view.contact && 
                       typeof(contact) != "undefined") ? (telephony.view.contact == contact) : false
        }

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
