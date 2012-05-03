import QtQuick 1.1

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

        leftIconSource: text ? "assets/cross.png" : "assets/search_icon.png"
        onLeftIconClicked: text = ""
    }

    Flickable {
        contentHeight: favouriteContactList.height + contactList.height
        flickableDirection: Flickable.VerticalFlick
        anchors.top: contactsSearchBox.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5
        clip: true

        FavouriteContactList {
            id: favouriteContactList
            filter: searchQuery
            clip: true
            anchors.top: parent.top
            onContactClicked: contactsPanel.contactClicked(contact)
        }

        ContactList {
            id: contactList
            filter: searchQuery
            anchors.top: favouriteContactList.bottom
            clip: true
            onContactClicked: contactsPanel.contactClicked(contact)
        }
    }
}
