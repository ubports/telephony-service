import QtQuick 1.1
import QtMobility.contacts 1.1
import "../Widgets"
import "../ContactUtils"

DropDownSearch {
    id: search

    signal contactSelected(variant contact, string number)

    ContactModel {
        id: contacts
        manager: "folks"
        filter: ContactFilters {
            filterText: search.searchQuery
        }
    }
    // FIXME: references to runtime and fake model need to be removed before final release
    model: runtime ? fakeContacts : contacts

    delegate: ContactsSearchDelegate {
        anchors.left: parent ? parent.left : undefined
        anchors.right: parent ? parent.right : undefined

        onContactClicked: {
            search.text = number
            search.state = "idle";
            search.contactSelected(contact, number);
        }
    }

    listMaximumHeight: 300
}
