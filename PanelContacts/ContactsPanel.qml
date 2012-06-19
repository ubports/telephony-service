import QtQuick 1.1
import QtMobility.contacts 1.1
import ".."
import "../Widgets"
import "../fontUtils.js" as Font
import "../ContactUtils"

Item {
    id: contactsPanel

    property alias searchQuery : contactsSearchBox.searchQuery

    anchors.fill: parent
    signal contactClicked(variant contact)
    onContactClicked: telephony.showContactDetails(contactsModel, contact)

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

    ListItem {
        id: newContact

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: contactsSearchBox.bottom
        anchors.topMargin: 10

        topSeparator: true
        isIcon: true
        iconSource: "../assets/add_contacts_icon.png"
        text: "Add a new contact"
        onClicked: telephony.createNewContact(contactsModel)
        selected: telephony.contactDetails.loaded && telephony.view.added
    }

    ContactModel {
        id: contactsModel
        manager: "folks"

        filter: ContactFilters {
            filterText: contactsSearchBox.text
        }

        sortOrders: [
            SortOrder {
               detail: ContactDetail.DisplayLabel
               field: DisplayLabel.label
               direction:Qt.AscendingOrder
            }
        ]
    }

    ListView {
        id: contactsList
        anchors.top: newContact.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true
        // FIXME: references to runtime and fake model need to be removed before final release
        model: typeof(runtime) != "undefined" ? fakeContacts : contactsModel

        delegate: ContactDelegate {
            onClicked: contactsPanel.contactClicked(contact)
            selected: telephony.contactDetails.loaded && telephony.view.contact == contact
        }

        section.property: "display"
        section.criteria: ViewSection.FirstCharacter
        section.delegate: TextCustom {
            width: parent.width
            height: paintedHeight
            text: section
            font.pixelSize: Font.sizeToPixels("medium")
            font.bold: true
            Rectangle {
                height: 1
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                color: "black"
            }
        }
    }
}
