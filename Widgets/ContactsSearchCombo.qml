import QtQuick 1.1
import TelephonyApp 0.1
import "../Widgets"

DropDownSearch {
    id: search

    signal contactSelected(variant contact, string number)

    ContactProxyModel {
        id: contacts
        model: contactModel
        filterText: search.text != "" ? search.text : "some text that won't match"
    }

    // FIXME: references to runtime and fake model need to be removed before final release
    model: typeof(runtime) != "undefined" ? fakeContacts : contacts

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
