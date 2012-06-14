import QtQuick 1.1
import QtMobility.contacts 1.1
import ".."
import "../Widgets"
import "../fontUtils.js" as Font
import "../ContactUtils"

Rectangle {
    id: contactsPanel

    property alias searchQuery : contactsSearchBox.searchQuery

    anchors.fill: parent
    signal contactClicked(variant contact)
    onContactClicked: telephony.showContactDetails(contact)

    SearchEntry {
        id: contactsSearchBox
        height: 30
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5

        leftIconSource: text ? "../assets/cross.png" : "../assets/search_icon.png"
        onLeftIconClicked: text = ""
    }

    Button {
        id: newContact
        height: 45
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: contactsSearchBox.bottom

        icon: "../assets/icon_plus.png"
        iconWidth: 35
        text: "Add a new contact"
        onClicked: telephony.createNewContact(contactsModel)
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
        anchors.margins: 5
        clip: true
        // FIXME: references to runtime and fake model need to be removed before final release
        model: typeof(runtime) != "undefined" ? fakeContacts : contactsModel

        delegate: ContactDelegate {
            onClicked: contactsPanel.contactClicked(contact)
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
